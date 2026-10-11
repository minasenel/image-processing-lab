// Hafta 3 - gorev_1, gorev_2, gorev_3 icin Google Colab CUDA dongulu sure olcumu
// Colab'de bir hucreye yapistirip calistirin (/content/image2.png yuklu olmali).

%%bash
cat << 'EOF' > gpu_benchmark.cpp
#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <functional>
#include <numeric>
#include <vector>
#include <opencv2/core.hpp>
#include <opencv2/core/cuda.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/cudaimgproc.hpp>
#include <opencv2/cudaarithm.hpp>

using Clock = std::chrono::steady_clock;

struct Stats { double first, mean, median, min, total; };

// fn her cagrida GPU isini bitirip senkronize olmali
static Stats measure(const std::function<void()>& fn, int warmup, int iters) {
    auto us = [](Clock::time_point a, Clock::time_point b) {
        return std::chrono::duration<double, std::micro>(b - a).count();
    };
    auto t0 = Clock::now();
    fn();                                   // ilk (soguk) cagri: kernel yukleme vb.
    double first = us(t0, Clock::now());
    for (int i = 0; i < warmup; ++i) fn();

    std::vector<double> t(iters);
    auto loop_start = Clock::now();
    for (int i = 0; i < iters; ++i) {
        auto a = Clock::now();
        fn();
        t[i] = us(a, Clock::now());
    }
    double total = us(loop_start, Clock::now());
    std::vector<double> s = t;
    std::sort(s.begin(), s.end());
    double mean = std::accumulate(t.begin(), t.end(), 0.0) / iters;
    return {first, mean, s[iters / 2], s[0], total};
}

int main(int argc, char** argv) {
    const int iters = argc > 1 ? std::atoi(argv[1]) : 300;
    const int warmup = 10;

    cv::Mat h_src = cv::imread("/content/image2.png", cv::IMREAD_GRAYSCALE);
    if (h_src.empty()) { std::fprintf(stderr, "[HATA] /content/image2.png yok\n"); return 1; }

    cv::cuda::DeviceInfo dev;
    std::printf("[BILGI] Girdi: %dx%d | GPU: %s | iterasyon: %d\n\n",
                h_src.cols, h_src.rows, dev.name(), iters);

    // CUDA context'i olcum disinda baslat (orijinal tek olcumde bu maliyet de vardi)
    auto c0 = Clock::now();
    cv::cuda::GpuMat d_src;
    d_src.upload(h_src);
    cv::cuda::Stream::Null().waitForCompletion();
    double ctx_us = std::chrono::duration<double, std::micro>(Clock::now() - c0).count();
    std::printf("[BILGI] CUDA context + ilk upload: %.1f us\n\n", ctx_us);

    // Ara tamponlar dongu disinda: ilk cagridan sonra yeniden tahsis edilmez
    cv::cuda::GpuMat d_float, d_sub, d_scaled, d_out, d_tmp;
    cv::Mat h_out;
    cv::Ptr<cv::cuda::CLAHE> clahe = cv::cuda::createCLAHE(2.0, cv::Size(8, 8));

    // gorev_1: orijinal koddaki adimlarin aynisi
    auto stretch = [&](const cv::cuda::GpuMat& in) {
        double mn = 0, mx = 0;
        cv::cuda::minMaxLoc(in, &mn, &mx, nullptr, nullptr);
        in.convertTo(d_float, CV_32FC1);
        cv::cuda::subtract(d_float, cv::Scalar(mn), d_sub);
        cv::cuda::multiply(d_sub, cv::Scalar(255.0 / (mx - mn)), d_scaled);
        d_scaled.convertTo(d_out, CV_8UC1);
    };
    auto sync = [] { cv::cuda::Stream::Null().waitForCompletion(); };

    struct Task { const char* name; std::function<void()> fn; };
    std::vector<Task> tasks = {
        {"gorev_1 germe (sadece kernel)",       [&] { stretch(d_src); sync(); }},
        {"gorev_1 germe (upload+kernel+down)",  [&] { d_tmp.upload(h_src); stretch(d_tmp); d_out.download(h_out); }},
        {"gorev_2 equalizeHist (sadece kernel)",[&] { cv::cuda::equalizeHist(d_src, d_out); sync(); }},
        {"gorev_2 equalizeHist (up+k+down)",    [&] { d_tmp.upload(h_src); cv::cuda::equalizeHist(d_tmp, d_out); d_out.download(h_out); }},
        {"gorev_3 CLAHE (sadece kernel)",       [&] { clahe->apply(d_src, d_out); sync(); }},
        {"gorev_3 CLAHE (up+k+down)",           [&] { d_tmp.upload(h_src); clahe->apply(d_tmp, d_out); d_out.download(h_out); }},
    };

    std::printf("%-38s %10s %10s %10s %10s %12s\n",
                "Islem", "Ilk(us)", "Ort(us)", "Medyan", "Min", "Toplam(us)");
    for (const auto& task : tasks) {
        Stats st = measure(task.fn, warmup, iters);
        std::printf("%-38s %10.1f %10.1f %10.1f %10.1f %12.1f\n",
                    task.name, st.first, st.mean, st.median, st.min, st.total);
    }
    return 0;
}
EOF

g++ -std=c++17 -O3 gpu_benchmark.cpp -o gpu_benchmark \
    -I/content/opencv_install/include/opencv4 \
    -L/content/opencv_install/lib \
    -lopencv_core -lopencv_imgcodecs -lopencv_cudaimgproc -lopencv_cudaarithm \
    -Wl,-rpath,/content/opencv_install/lib

./gpu_benchmark 300
