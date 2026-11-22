"""
Hafta 9 - Gorev 6: Hareket tabanli ROI icinde ten rengi tespiti

1. Webcam'den ardisik kareler okunur.
2. Iki kare arasindaki fark (frame differencing) ile hareketli bolge bulunur.
   Yeterince piksel degismediyse o kare "durgun" sayilir, hareketli ROI yoktur.
3. Hareketli bolgenin disi arka plan olarak kabul edilir ve islenmez.
4. Sadece ROI (hareketli bolge) icinde YCrCb renk uzayinda ten rengi esiklenir
   ve tespit edilen pikseller kirmiziya boyanir.

Cikis: 'q' tusuna basarak kapat.
"""

import cv2
import numpy as np

# --- Ayarlanabilir parametreler ---
DIFF_THRESH = 25          # iki kare arasindaki piksel farki esigi
MIN_MOTION_PIXELS = 1500   # ROI olusturmak icin gereken minimum hareketli piksel sayisi
MORPH_KERNEL = cv2.getStructuringElement(cv2.MORPH_ELLIPSE, (7, 7))

# Literaturde yaygin ten rengi esikleri (YCrCb)
SKIN_LOWER = np.array((0, 133, 77), dtype=np.uint8)
SKIN_UPPER = np.array((255, 173, 127), dtype=np.uint8)


def get_motion_mask(prev_gray, curr_gray):
    diff = cv2.absdiff(prev_gray, curr_gray)
    _, motion_mask = cv2.threshold(diff, DIFF_THRESH, 255, cv2.THRESH_BINARY)
    motion_mask = cv2.morphologyEx(motion_mask, cv2.MORPH_OPEN, MORPH_KERNEL)
    motion_mask = cv2.morphologyEx(motion_mask, cv2.MORPH_CLOSE, MORPH_KERNEL)
    motion_mask = cv2.dilate(motion_mask, MORPH_KERNEL, iterations=2)
    return motion_mask


def get_motion_roi(motion_mask):
    """Hareketli pikselleri iceren en buyuk sinirlayici kutuyu (bounding box) dondurur."""
    contours, _ = cv2.findContours(motion_mask, cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)
    if not contours:
        return None

    # Tum hareketli konturlari birlestiren tek bir sinirlayici kutu
    x_min, y_min, x_max, y_max = None, None, None, None
    total_area = 0
    for c in contours:
        area = cv2.contourArea(c)
        if area < 150:  # kucuk gurultu konturlarini atla
            continue
        total_area += area
        x, y, w, h = cv2.boundingRect(c)
        x_min = x if x_min is None else min(x_min, x)
        y_min = y if y_min is None else min(y_min, y)
        x_max = x + w if x_max is None else max(x_max, x + w)
        y_max = y + h if y_max is None else max(y_max, y + h)

    if x_min is None or total_area < MIN_MOTION_PIXELS:
        return None

    return x_min, y_min, x_max, y_max


def detect_skin_in_roi(frame_bgr, roi):
    """ROI icinde YCrCb ten rengi maskesi uretir, tam kare boyutunda mask dondurur."""
    x1, y1, x2, y2 = roi
    full_mask = np.zeros(frame_bgr.shape[:2], dtype=np.uint8)

    roi_bgr = frame_bgr[y1:y2, x1:x2]
    if roi_bgr.size == 0:
        return full_mask

    roi_ycrcb = cv2.cvtColor(roi_bgr, cv2.COLOR_BGR2YCrCb)
    roi_mask = cv2.inRange(roi_ycrcb, SKIN_LOWER, SKIN_UPPER)
    roi_mask = cv2.morphologyEx(roi_mask, cv2.MORPH_OPEN, MORPH_KERNEL)
    roi_mask = cv2.morphologyEx(roi_mask, cv2.MORPH_CLOSE, MORPH_KERNEL)

    full_mask[y1:y2, x1:x2] = roi_mask
    return full_mask


def main():
    cap = cv2.VideoCapture(0)
    if not cap.isOpened():
        print("[HATA] Webcam acilamadi.")
        return

    ok, prev_frame = cap.read()
    if not ok:
        print("[HATA] Webcam'den kare okunamadi.")
        return
    prev_gray = cv2.cvtColor(prev_frame, cv2.COLOR_BGR2GRAY)
    prev_gray = cv2.GaussianBlur(prev_gray, (5, 5), 0)

    print("Cikmak icin 'q' tusuna basin.")

    while True:
        ok, frame = cap.read()
        if not ok:
            break

        gray = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)
        gray = cv2.GaussianBlur(gray, (5, 5), 0)

        motion_mask = get_motion_mask(prev_gray, gray)
        roi = get_motion_roi(motion_mask)

        display = frame.copy()

        if roi is not None:
            x1, y1, x2, y2 = roi
            skin_mask = detect_skin_in_roi(frame, roi)

            # Ten tespit edilen pikselleri kirmiziya boya (sadece ROI icinde)
            display[skin_mask > 0] = (0, 0, 255)

            # ROI'yi gostermek icin yesil cerceve ciz
            cv2.rectangle(display, (x1, y1), (x2, y2), (0, 255, 0), 2)
            cv2.putText(display, "HAREKET VAR - ROI icinde ten taraniyor", (10, 25),
                        cv2.FONT_HERSHEY_SIMPLEX, 0.6, (0, 255, 0), 2)
        else:
            cv2.putText(display, "Hareket yok (arka plan)", (10, 25),
                        cv2.FONT_HERSHEY_SIMPLEX, 0.6, (0, 0, 255), 2)

        cv2.imshow("Webcam - Hareket + Ten Tespiti", display)
        cv2.imshow("Hareket Maskesi", motion_mask)

        prev_gray = gray

        if cv2.waitKey(1) & 0xFF == ord('q'):
            break

    cap.release()
    cv2.destroyAllWindows()


if __name__ == "__main__":
    main()
