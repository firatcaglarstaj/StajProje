import socket
import json
import struct
import base64
import time
import cv2
import numpy as np
from ultralytics import YOLO

class TiledDroneYOLOService:
    """
    C++ Qt uygulamasından TCP üzerinden video kareleri alıp,
    bu kareleri dilimleyerek YOLO ile işleyen ve sonuçları geri gönderen servis.
    """
    def __init__(self, host='localhost', port=8888):
        print(">>> YOLO Service başlatılıyor...")

        # --- MODEL YÜKLEME ---
        # Kendi eğittiğiniz modelin .pt dosyasını buraya yazın
        try:
            self.model = YOLO('best.pt')
            print(f">>> YOLO modeli ('best.pt') başarıyla yüklendi.")
            print(f">>> Modelin tanıdığı sınıflar: {self.model.names}")
        except Exception as e:
            print(f"!!! Model yükleme hatası: {e}")
            print("!!! Lütfen model dosyasının doğru yolda olduğundan emin olun.")
            self.model = None

        # --- SUNUCU AYARLARI ---
        self.host = host
        self.port = port
        self.socket = None

        # --- İSTATİSTİKLER ---
        self.frame_count = 0
        self.total_detection_count = 0

    def start_server(self):
        """TCP sunucusunu başlatır ve C++ istemcisinden bağlantı bekler."""
        if self.model is None:
            print("!!! Model yüklenemediği için sunucu başlatılamıyor.")
            return

        try:
            self.socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            # Adresin tekrar kullanılabilmesi için soket ayarı
            self.socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
            self.socket.bind((self.host, self.port))
            self.socket.listen(1)

            print(f"\n>>> Sunucu başlatıldı ve dinlemede: {self.host}:{self.port}")
            print(">>> C++ uygulamasından bağlantı bekleniyor...")

            while True:
                client_socket, client_address = self.socket.accept()
                print(f"\n>>> C++ istemcisi bağlandı: {client_address}")
                self.process_frames(client_socket)
                print(f">>> C++ istemcisinin bağlantısı kapandı: {client_address}")

        except Exception as e:
            print(f"!!! Sunucu hatası: {e}")
        finally:
            if self.socket:
                self.socket.close()
                print(">>> Sunucu kapatıldı.")
            self.print_stats()

    def process_frames(self, client_socket):
        """Bağlı istemciden gelen kareleri sürekli olarak işler."""
        try:
            while True:
                message = self.receive_message(client_socket)
                if not message:
                    # Boş mesaj, bağlantının kapandığı anlamına gelir
                    break

                if message.get('type') == 'frame_request':
                    self.handle_frame(client_socket, message)
                else:
                    print(f"??? Bilinmeyen mesaj tipi alındı: {message.get('type')}")

        except ConnectionResetError:
            print("!!! İstemci bağlantıyı aniden kapattı.")
        except Exception as e:
            print(f"!!! Kare işleme döngüsünde hata: {e}")
        finally:
            client_socket.close()

    def receive_message(self, client_socket):
        """C++ istemcisinden gelen, başında 4 byte boyut bilgisi olan bir mesajı okur."""
        try:
            # İlk 4 byte: Mesajın toplam boyutunu belirtir (Big-Endian formatında)
            size_data = client_socket.recv(4)
            if not size_data:
                return None
            message_size = struct.unpack('>I', size_data)[0]

            # Gelen veriyi parçalar halinde oku
            json_data = b''
            while len(json_data) < message_size:
                chunk = client_socket.recv(message_size - len(json_data))
                if not chunk:
                    return None
                json_data += chunk

            message = json.loads(json_data.decode('utf-8'))
            return message
        except (struct.error, json.JSONDecodeError, ConnectionResetError) as e:
            print(f"!!! Mesaj alma veya parse etme hatası: {e}")
            return None

    def send_message(self, client_socket, message):
        """Verilen mesajı JSON formatına çevirip C++ istemcisine gönderir."""
        try:
            json_data = json.dumps(message).encode('utf-8')
            # Mesajın başına 4 byte'lık boyut bilgisini ekle
            size = struct.pack('>I', len(json_data))
            client_socket.sendall(size + json_data)
        except Exception as e:
            print(f"!!! Mesaj gönderme hatası: {e}")

    def decode_frame(self, base64_data):
        """Base64 string'ini bir OpenCV görüntüsüne (frame) dönüştürür."""
        try:
            image_data = base64.b64decode(base64_data)
            nparr = np.frombuffer(image_data, np.uint8)
            frame = cv2.imdecode(nparr, cv2.IMREAD_COLOR)
            return frame
        except Exception as e:
            print(f"!!! Frame decode hatası: {e}")
            return None

    def perform_inference_on_slices(self, frame):
        """Yüksek çözünürlüklü bir kareyi dilimler, tespit yapar ve sonuçları birleştirir."""
        tile_size = 640
        overlap = 256
        stride = tile_size - overlap
        conf_threshold = 0.15  # Tespit için minimum güven skoru
        nms_threshold = 0.5   # Mükerrer kutuları silmek için örtüşme eşiği

        frame_height, frame_width, _ = frame.shape
        all_detections = []

        for y in range(0, frame_height, stride):
            for x in range(0, frame_width, stride):
                y_end = min(y + tile_size, frame_height)
                x_end = min(x + tile_size, frame_width)
                tile = frame[y:y_end, x:x_end]

                if tile.shape[0] < overlap or tile.shape[1] < overlap:
                    continue

                results = self.model(tile, verbose=False, conf=conf_threshold)

                for r in results:
                    for box in r.boxes:
                        x1_local, y1_local, x2_local, y2_local = box.xyxy[0].tolist()
                        all_detections.append({
                            'bbox': [x + x1_local, y + y1_local, x + x2_local, y + y2_local],
                            'confidence': float(box.conf[0]),
                            'class_id': int(box.cls[0]),
                            'class_name': self.model.names[int(box.cls[0])]
                        })

        if not all_detections:
            return []

        bboxes_for_nms = np.array([d['bbox'] for d in all_detections])
        scores_for_nms = np.array([d['confidence'] for d in all_detections])

        indices = cv2.dnn.NMSBoxes(bboxes_for_nms.tolist(), scores_for_nms.tolist(), score_threshold=conf_threshold, nms_threshold=nms_threshold)

        final_detections = []
        if len(indices) > 0:
            for i in indices.flatten():
                det = all_detections[i]
                final_detections.append({
                    'class_id': det['class_id'],
                    'class_name': det['class_name'],
                    'confidence': det['confidence'],
                    'bbox': {
                        'x1': int(det['bbox'][0]),
                        'y1': int(det['bbox'][1]),
                        'x2': int(det['bbox'][2]),
                        'y2': int(det['bbox'][3])
                    }
                })
        return final_detections

    def handle_frame(self, client_socket, message):
        """Gelen bir 'frame_request' mesajını işler."""
        try:
            payload = message['payload']
            frame_id = payload['frame_id']
            print(f"--> Frame #{frame_id} işleniyor (dilimleme ile)...")
            start_time = time.time()

            frame = self.decode_frame(payload['data'])
            if frame is None:
                return

            detections = self.perform_inference_on_slices(frame)

            processing_time_ms = (time.time() - start_time) * 1000

            response = {
                'type': 'detection_result',
                'payload': {
                    'frame_id': frame_id,
                    'detections': detections,
                    'processing_time_ms': processing_time_ms
                }
            }
            self.send_message(client_socket, response)

            self.frame_count += 1
            self.total_detection_count += len(detections)

            print(f"<-- Frame #{frame_id}: {len(detections)} nesne bulundu. Süre: {processing_time_ms:.1f} ms")

        except Exception as e:
            print(f"!!! Frame #{message['payload']['frame_id']} işlenirken hata oluştu: {e}")

    def print_stats(self):
        """Oturum sonundaki istatistikleri yazdırır."""
        print("\n--- OTURUM İSTATİSTİKLERİ ---")
        print(f"  Toplam İşlenen Kare: {self.frame_count}")
        print(f"  Toplam Tespit Edilen Nesne: {self.total_detection_count}")
        if self.frame_count > 0:
            avg = self.total_detection_count / self.frame_count
            print(f"  Kare Başına Ortalama Nesne: {avg:.2f}")
        print("----------------------------\n")

if __name__ == "__main__":
    print("===================================")
    print("      YOLO Tespit Servisi v2.0     ")
    print("      (Dilimleme Destekli)       ")
    print("===================================")

    service = TiledDroneYOLOService()
    try:
        service.start_server()
    except KeyboardInterrupt:
        print("\n!!! Kullanıcı tarafından servis durduruldu (Ctrl+C).")
    except Exception as e:
        print(f"!!! Ana programda beklenmedik bir hata oluştu: {e}")
