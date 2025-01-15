# proje1
# Sayısal Saat Uygulaması

## Açıklama
Bu proje, Tiva C Launchpad veya benzeri bir mikrodenetleyici kullanarak bir **sayısal saat** gerçekleştirmeyi amaçlamaktadır. Zaman bilgisi bir **16x2 LCD ekran** üzerinde görüntülenir. Zaman takibi, bir **timer kesmesi** ile yapılır ve saniyeler ilerledikçe LED'in yanıp sönmesi sağlanır.

## Donanım Gereksinimleri
- Tiva C Launchpad (veya uyumlu bir mikrodenetleyici platformu)
- 16x2 LCD ekran
- GPIO bağlantıları (RS, E, D4-D7 için)

## Kullanılan Kütüphaneler
- **TivaWare Library**
  - `hw_types.h`, `hw_memmap.h`, `hw_ints.h`: Donanım tanımlamaları.
  - `sysctl.h`: Sistem saat ayarları.
  - `gpio.h`: GPIO kontrolü.
  - `timer.h`: Timer yapılandırma ve kesme işlemleri.
  - `interrupt.h`: Kesme işlemleri.

## Bağlantılar
Aşağıdaki pin bağlantıları gereklidir:
- **LCD Kontrol Pinleri:**
  - RS: PB0
  - E: PB1
  - D4: PB4
  - D5: PB5
  - D6: PB6
  - D7: PB7
- **LED Çıkışı:**
  - PF2 (Tiva C'deki yerleşik mavi LED)

## Yazılım Özellikleri
- **Zaman Takibi:** 
  - Timer kesmesi ile saat, dakika ve saniye güncellenir.
  - 24 saat formatında zaman görüntülenir.
- **LCD Kontrolü:**
  - LCD ekran, 4-bit modda çalıştırılır.
  - Güncel saat bilgisi `hh:mm:ss` formatında gösterilir.
- **LED Durumu:**
  - Kesme fonksiyonunda LED'in durumu her saniyede bir değiştirilir.

## Kodun Çalıştırılması
1. TivaWare kütüphanesini mikrodenetleyicinizin ortamına ekleyin.
2. Yukarıdaki kodu bir IDE'ye (ör. Code Composer Studio veya Keil uVision) yapıştırın.
3. Projeyi derleyip mikrodenetleyiciye yükleyin.

## Fonksiyonlar
### 1. `SetInitSettings`
- Timer ve GPIO ayarlarını yapar.
- Timer kesmesini ve periyodik modu etkinleştirir.

### 2. `timerkesmefonksiyonu`
- Zamanı günceller (saniye, dakika ve saat).
- Formatlanmış zaman bilgisini LCD'ye yazdırır.
- LED'in durumunu değiştirir.

### 3. `lcd_init`
- LCD'yi başlatır ve 4-bit modda çalışacak şekilde yapılandırır.

### 4. `format_time`
- Saat bilgisini `hh:mm:ss` formatına çevirir.

### 5. `lcd_print`
- Verilen bir stringi LCD'ye yazdırır.

### 6. `lcd_command` ve `lcd_data`
- LCD'ye komut veya veri gönderir.

### 7. `delayMs`
- Gecikme süresi sağlar (milisaniye cinsinden).

## Örnek Çıktı
16x2 LCD ekranın ikinci satırında saat bilgisi aşağıdaki gibi gösterilecektir:
