// Hafta 3 - gorev_1, gorev_2, gorev_3 icin yerel CPU (cok is parcacikli) sure olcumu
// Derleme (macOS / Homebrew OpenCV 5):
//   clang++ -std=c++17 -O3 cpu_benchmark.cpp -o cpu_benchmark $(pkg-config --cflags --libs opencv5)
// Calistirma:
//   ./cpu_benchmark ../image2.png [iterasyon=300]

#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <functional>
#include <iostream>
#include <mutex>
#include <numeric>
#include <string>
#include <vector>
#include <opencv2/core.hpp>
#include <opencv2/core/utility.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>

using Clock = std::chrono::steady_clock;

// ---------------------------------------------------------------------------
// gorev_1: Kontrast germe. GPU kodundaki adimlarin birebir CPU karsiligi.
// (minMaxLoc, convertTo, subtract, multiply, convertTo) -> OpenCV bu
// fonksiyonlari kendi thread havuzunda paralel calistirir.
// ---------------------------------------------------------------------------
static cv::Mat stretch_opencv(const cv::Mat& src) {
    double min_val = 0.0, max_val = 0.0;
    cv::minMaxLoc(src, &min_val, &max_val);
    cv::Mat f, sub, scaled, dst;
    src.convertTo(f, CV_32FC1);
    cv::subtract(f, cv::Scalar(min_val), sub);
    float scale = 255.0f / static_cast<float>(max_val - min_val);
    cv::multiply(sub, cv::Scalar(scale), scaled);
    scaled.convertTo(dst, CV_8UC1);
    return dst;
}

// gorev_1 manuel paralel versiyon: satir bloklarinda min/max, sonra tek gecis.
static cv::Mat stretch_manual(const cv::Mat& src) {
    const int rows = src.rows, cols = src.cols;
    int min_v = 255, max_v = 0;
    std::mutex m;
    cv::parallel_for_(cv::Range(0, rows), [&](const cv::Range& rg) {
        int lmin = 255, lmax = 0;
        for (int r = rg.start; r < rg.end; ++r) {
            const uchar* p = src.ptr<uchar>(r);
            for (int c = 0; c < cols; ++c) {
                lmin = std::min<int>(lmin, p[c]);
                lmax = std::max<int>(lmax, p[c]);
            }
        }
        std::lock_guard<std::mutex> lk(m);
        min_v = std::min(min_v, lmin);
        max_v = std::max(max_v, lmax);
    });
    const float scale = 255.0f / static_cast<float>(max_v - min_v);
    cv::Mat dst(rows, cols, CV_8UC1);
    cv::parallel_for_(cv::Range(0, rows), [&](const cv::Range& rg) {
        for (int r = rg.start; r < rg.end; ++r) {
            const uchar* s = src.ptr<uchar>(r);
            uchar* d = dst.ptr<uchar>(r);
            for (int c = 0; c < cols; ++c)
                d[c] = cv::saturate_cast<uchar>((s[c] - min_v) * scale);
        }
    });
    return dst;
}

// ---------------------------------------------------------------------------
// gorev_2: Histogram esitleme
// ---------------------------------------------------------------------------
static cv::Mat equalize_builtin(const cv::Mat& src) {
    cv::Mat dst;
    cv::equalizeHist(src, dst);
    return dst;
}

// Orijinal gorev_2_custom algoritmasi; histogram ve LUT uygulama adimlari paralel.
static cv::Mat equalize_custom(const cv::Mat& src) {
    const int rows = src.rows, cols = src.cols;
    const int total_pixels = rows * cols;

    // 1. Histogram: her blok kendi yerel histogramini sayar, sonra birlestirilir
    std::vector<int> hist(256, 0);
    std::mutex m;
    cv::parallel_for_(cv::Range(0, rows), [&](const cv::Range& rg) {
        std::array<int, 256> local{};
        for (int r = rg.start; r < rg.end; ++r) {
            const uchar* row = src.ptr<uchar>(r);
            for (int c = 0; c < cols; ++c) local[row[c]]++;
        }
        std::lock_guard<std::mutex> lk(m);
        for (int i = 0; i < 256; ++i) hist[i] += local[i];
    });

    // 2. CDF
    std::vector<int> cdf(256, 0);
    cdf[0] = hist[0];
    for (int i = 1; i < 256; ++i) cdf[i] = cdf[i - 1] + hist[i];
    int cdf_min = 0;
    for (int i = 0; i < 256; ++i)
        if (cdf[i] > 0) { cdf_min = cdf[i]; break; }

    // 3. LUT
    std::vector<uchar> lut(256, 0);
    const int denominator = total_pixels - cdf_min;
    for (int i = 0; i < 256; ++i) {
        if (cdf[i] < cdf_min || denominator <= 0) lut[i] = 0;
        else lut[i] = static_cast<uchar>(std::round(
                 (static_cast<float>(cdf[i] - cdf_min) / denominator) * 255.0f));
    }

    // 4. Pikselleri guncelle (paralel)
    cv::Mat dst(rows, cols, CV_8UC1);
    cv::parallel_for_(cv::Range(0, rows), [&](const cv::Range& rg) {
        for (int r = rg.start; r < rg.end; ++r) {
            const uchar* s = src.ptr<uchar>(r);
            uchar* d = dst.ptr<uchar>(r);
            for (int c = 0; c < cols; ++c) d[c] = lut[s[c]];
        }
    });
    return dst;
}

// ---------------------------------------------------------------------------
// gorev_3: CLAHE (clipLimit = 2.0, grid = 8x8)
// ---------------------------------------------------------------------------
static cv::Mat clahe_builtin(const cv::Mat& src) {
    cv::Mat dst;
    cv::Ptr<cv::CLAHE> clahe = cv::createCLAHE(2.0, cv::Size(8, 8));
    clahe->apply(src, dst);
    return dst;
}

// Orijinal clahe_custom algoritmasi; 64 blogun LUT hesabi ve satir bazli
// bilinear enterpolasyon paralel. Piksel basina mantik birebir ayni.
static cv::Mat clahe_custom(const cv::Mat& src) {
    const int width = src.cols, height = src.rows;
    const int grid_x = 8, grid_y = 8;
    const double clip_limit_ratio = 2.0;

    std::vector<int> x_starts(grid_x), x_ends(grid_x);
    std::vector<int> y_starts(grid_y), y_ends(grid_y);
    std::vector<double> x_centers(grid_x), y_centers(grid_y);
    for (int i = 0; i < grid_x; ++i) {
        x_starts[i] = i * width / grid_x;
        x_ends[i] = (i + 1) * width / grid_x;
        x_centers[i] = (x_starts[i] + x_ends[i] - 1) / 2.0;
    }
    for (int j = 0; j < grid_y; ++j) {
        y_starts[j] = j * height / grid_y;
        y_ends[j] = (j + 1) * height / grid_y;
        y_centers[j] = (y_starts[j] + y_ends[j] - 1) / 2.0;
    }

    std::vector<std::vector<std::vector<uchar>>> luts(
        grid_y, std::vector<std::vector<uchar>>(grid_x, std::vector<uchar>(256)));

    // Her blok bagimsiz -> 64 blok paralel islenir
    cv::parallel_for_(cv::Range(0, grid_x * grid_y), [&](const cv::Range& rg) {
        for (int idx = rg.start; idx < rg.end; ++idx) {
            const int gy = idx / grid_x, gx = idx % grid_x;
            const int block_w = x_ends[gx] - x_starts[gx];
            const int block_h = y_ends[gy] - y_starts[gy];
            const int total_p = block_w * block_h;

            std::vector<int> hist(256, 0);
            for (int r = y_starts[gy]; r < y_ends[gy]; ++r) {
                const uchar* row = src.ptr<uchar>(r);
                for (int c = x_starts[gx]; c < x_ends[gx]; ++c) hist[row[c]]++;
            }

            int clip_val = std::max(1, static_cast<int>(clip_limit_ratio * (total_p / 256.0)));
            int excess = 0;
            for (int i = 0; i < 256; ++i) {
                if (hist[i] > clip_val) {
                    excess += (hist[i] - clip_val);
                    hist[i] = clip_val;
                }
            }
            int bonus = excess / 256, remainder = excess % 256;
            for (int i = 0; i < 256; ++i) {
                hist[i] += bonus;
                if (i < remainder) hist[i]++;
            }

            int sum = 0;
            for (int i = 0; i < 256; ++i) {
                sum += hist[i];
                luts[gy][gx][i] = static_cast<uchar>(
                    std::round((static_cast<float>(sum) / total_p) * 255.0f));
            }
        }
    });

    cv::Mat dst(height, width, CV_8UC1);
    cv::parallel_for_(cv::Range(0, height), [&](const cv::Range& rg) {
        for (int r = rg.start; r < rg.end; ++r) {
            uchar* dst_row = dst.ptr<uchar>(r);
            const uchar* src_row = src.ptr<uchar>(r);

            int y1 = 0, y2 = 0;
            double wy = 0.0;
            if (r <= y_centers[0]) {
                y1 = y2 = 0;
            } else if (r >= y_centers[grid_y - 1]) {
                y1 = y2 = grid_y - 1;
            } else {
                for (int j = 0; j < grid_y - 1; ++j) {
                    if (r >= y_centers[j] && r < y_centers[j + 1]) {
                        y1 = j; y2 = j + 1;
                        wy = (r - y_centers[j]) / (y_centers[j + 1] - y_centers[j]);
                        break;
                    }
                }
            }

            for (int c = 0; c < width; ++c) {
                uchar val = src_row[c];
                int x1 = 0, x2 = 0;
                double wx = 0.0;
                if (c <= x_centers[0]) {
                    x1 = x2 = 0;
                } else if (c >= x_centers[grid_x - 1]) {
                    x1 = x2 = grid_x - 1;
                } else {
                    for (int i = 0; i < grid_x - 1; ++i) {
                        if (c >= x_centers[i] && c < x_centers[i + 1]) {
                            x1 = i; x2 = i + 1;
                            wx = (c - x_centers[i]) / (x_centers[i + 1] - x_centers[i]);
                            break;
                        }
                    }
                }
                double v11 = luts[y1][x1][val], v12 = luts[y1][x2][val];
                double v21 = luts[y2][x1][val], v22 = luts[y2][x2][val];
                double top = (1.0 - wx) * v11 + wx * v12;
                double bottom = (1.0 - wx) * v21 + wx * v22;
                dst_row[c] = static_cast<uchar>(std::round((1.0 - wy) * top + wy * bottom));
            }
        }
    });
    return dst;
}

// ---------------------------------------------------------------------------
// Olcum altyapisi
// ---------------------------------------------------------------------------
struct Stats { double first, mean, median, min, stddev; };

static Stats measure(const std::function<cv::Mat()>& fn, int warmup, int iters) {
    auto us = [](Clock::time_point a, Clock::time_point b) {
        return std::chrono::duration<double, std::micro>(b - a).count();
    };
    auto t0 = Clock::now();
    cv::Mat sink = fn();                       // ilk (soguk) cagri
    double first = us(t0, Clock::now());
    for (int i = 0; i < warmup; ++i) sink = fn();

    std::vector<double> t(iters);
    for (int i = 0; i < iters; ++i) {
        auto a = Clock::now();
        sink = fn();
        t[i] = us(a, Clock::now());
    }
    std::vector<double> s = t;
    std::sort(s.begin(), s.end());
    double mean = std::accumulate(t.begin(), t.end(), 0.0) / iters;
    double var = 0.0;
    for (double v : t) var += (v - mean) * (v - mean);
    return {first, mean, s[iters / 2], s[0], std::sqrt(var / iters)};
}

int main(int argc, char** argv) {
    const std::string path = argc > 1 ? argv[1] : "../image2.png";
    const int iters = argc > 2 ? std::atoi(argv[2]) : 300;
    const int warmup = 10;

    cv::Mat src = cv::imread(path, cv::IMREAD_GRAYSCALE);
    if (src.empty()) { std::cerr << "[HATA] " << path << " okunamadi\n"; return 1; }

    const int hw_threads = cv::getNumberOfCPUs();
    std::printf("[BILGI] Girdi: %dx%d | CPU cekirdek: %d | OpenCV %s | iterasyon: %d\n\n",
                src.cols, src.rows, hw_threads, CV_VERSION, iters);

    struct Task { const char* name; std::function<cv::Mat()> fn; };
    std::vector<Task> tasks = {
        {"gorev_1 germe (OpenCV CPU adimlari)", [&] { return stretch_opencv(src); }},
        {"gorev_1 germe (manuel parallel_for_)", [&] { return stretch_manual(src); }},
        {"gorev_2 equalizeHist (hazir)",        [&] { return equalize_builtin(src); }},
        {"gorev_2 esitleme (manuel)",           [&] { return equalize_custom(src); }},
        {"gorev_3 CLAHE (hazir)",               [&] { return clahe_builtin(src); }},
        {"gorev_3 CLAHE (manuel)",              [&] { return clahe_custom(src); }},
    };

    std::printf("%-40s %8s %10s %10s %10s %10s %9s\n",
                "Islem", "Thread", "Ilk(us)", "Ort(us)", "Medyan", "Min", "Std");
    for (const auto& task : tasks) {
        for (int nt : {1, hw_threads}) {
            cv::setNumThreads(nt);
            Stats st = measure(task.fn, warmup, iters);
            std::printf("%-40s %8d %10.1f %10.1f %10.1f %10.1f %9.1f\n",
                        task.name, nt, st.first, st.mean, st.median, st.min, st.stddev);
        }
    }

    // Dogruluk kontrolu: paralel manuel ciktilar hazir fonksiyonlarla ayni mi?
    cv::setNumThreads(hw_threads);
    double d1, d2, d3;
    cv::Mat diff;
    cv::absdiff(stretch_manual(src), stretch_opencv(src), diff);
    cv::minMaxLoc(diff, nullptr, &d1);
    cv::absdiff(equalize_custom(src), equalize_builtin(src), diff);
    cv::minMaxLoc(diff, nullptr, &d2);
    cv::absdiff(clahe_custom(src), clahe_builtin(src), diff);
    cv::minMaxLoc(diff, nullptr, &d3);
    double mean3 = cv::mean(diff)[0];
    std::printf("\n[DOGRULAMA] max|fark| germe: %.0f | esitleme: %.0f | CLAHE: %.0f (ort %.3f)\n",
                d1, d2, d3, mean3);
    return 0;
}
