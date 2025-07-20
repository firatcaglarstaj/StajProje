# if __name__ == "__main__":
#     pass
import socket
import json
import struct
import base64
import cv2
import numpy as np
from ultralytics import YOLO
import time

class SimpleYOLOService:
    def __init__(self):
        print(" YOLO Service başlatılıyor...")

        # YOLO modelini yükle
        try:
            self.model = YOLO('yolo11n.pt')  # Nano model - hızlı
            print("✅ YOLO model yüklendi")
        except Exception as e:
            print(f"❌ Model yükleme hatası: {e}")
            self.model = None

        # Server ayarları
        self.host = 'localhost'
        self.port = 8888
        self.socket = None

        # Basit sayaçlar
        self.frame_count = 0
        self.detection_count = 0

    def start_server(self):
        """TCP server başlat ve C++'dan bağlantı bekle"""
        try:
            self.socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            self.socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
            self.socket.bind((self.host, self.port))
            self.socket.listen(1)

            print(f"🔗 Server başlatıldı: {self.host}:{self.port}")
            print("📡 C++ bağlantısı bekleniyor...")

            # C++'dan bağlantı bekle
            client_socket, client_address = self.socket.accept()
            print(f"C++ bağlandı: {client_address}")

            # Frame işleme döngüsü
            self.process_frames(client_socket)

        except Exception as e:
            print(f" Server hatası: {e}")
        finally:
            if self.socket:
                self.socket.close()
                print("🔌 Server kapatıldı")

    def process_frames(self, client_socket):
        """C++'dan gelen frame'leri işle"""
        print("🎬 Frame işleme başladı...")

        try:
            while True:
                # C++'dan mesaj al
                message = self.receive_message(client_socket)
                if not message:
                    break

                # Frame'i işle
                if message.get('type') == 'frame_request':
                    self.handle_frame(client_socket, message)

        except KeyboardInterrupt:
            print("\nKullanıcı tarafından durduruldu")
        except Exception as e:
            print(f"Frame işleme hatası: {e}")
        finally:
            client_socket.close()
            print(" C++ bağlantısı kapatıldı")

    def receive_message(self, client_socket):
        """C++'dan JSON mesaj al"""
        try:
            # İlk 4 byte = mesaj boyutu
            size_data = client_socket.recv(4)
            if not size_data:
                return None

            message_size = struct.unpack('>I', size_data)[0]

            # JSON mesajını al
            json_data = b''
            while len(json_data) < message_size:
                chunk = client_socket.recv(message_size - len(json_data))
                if not chunk:
                    return None
                json_data += chunk

            # JSON parse et
            message = json.loads(json_data.decode('utf-8'))
            return message

        except Exception as e:
            print(f" Mesaj alma hatası: {e}")
            return None

    def send_message(self, client_socket, message):
        """C++ a JSON mesaj gönder"""
        try:
            # JSON'a çevir
            json_data = json.dumps(message).encode('utf-8')

            # Boyut + JSON gönder
            size = struct.pack('>I', len(json_data))
            client_socket.send(size + json_data)

        except Exception as e:
            print(f"Mesaj gönderme hatası: {e}")

    def handle_frame(self, client_socket, message):
        """Frame'i YOLO ile işle ve sonuç gönder"""
        try:
            payload = message['payload']
            frame_id = payload['frame_id']

            print(f" Frame işleniyor: {frame_id}")

            # Base64'dan OpenCV frame'e çevir
            frame = self.decode_frame(payload['data'])
            if frame is None:
                print(" Frame decode edilemedi")
                return

            # YOLO inference
            detections = self.detect_objects(frame, frame_id)

            # Sonucu C++'a gönder
            response = {
                'type': 'detection_result',
                'payload': {
                    'frame_id': frame_id,
                    'detections': detections,
                    'processing_time_ms': 50  # Basit değer
                }
            }

            self.send_message(client_socket, response)

            # Sayaçları güncelle
            self.frame_count += 1
            self.detection_count += len(detections)

            print(f"Frame {frame_id}: {len(detections)} nesne bulundu")

        except Exception as e:
            print(f"Frame işleme hatası: {e}")

    def decode_frame(self, base64_data):
        """Base64'dan OpenCV frame'e çevir"""
        try:
            # Base64 decode
            image_data = base64.b64decode(base64_data)

            # NumPy array e çevir
            nparr = np.frombuffer(image_data, np.uint8)

            # OpenCV image e çevir
            frame = cv2.imdecode(nparr, cv2.IMREAD_COLOR)

            return frame

        except Exception as e:
            print(f" Frame decode hatası: {e}")
            return None

    def detect_objects(self, frame, frame_id):
        """YOLO ile nesne tespiti yap"""
        if self.model is None:
            print(" Model yüklü değil")
            return []

        try:
            # YOLO inference
            results = self.model(frame, verbose=False)

            detections = []

            # Sonuçları işle
            for r in results:
                boxes = r.boxes
                if boxes is not None:
                    for box in boxes:
                        # Bounding box koordinatları
                        x1, y1, x2, y2 = box.xyxy[0].tolist()

                        # Class ve confidence
                        class_id = int(box.cls[0])
                        confidence = float(box.conf[0])
                        class_name = self.model.names[class_id]

                        # Minimum confidence kontrolü
                        if confidence > 0.3:
                            detection = {
                                'class_id': class_id,
                                'class_name': class_name,
                                'confidence': confidence,
                                'bbox': {
                                    'x1': int(x1),
                                    'y1': int(y1),
                                    'x2': int(x2),
                                    'y2': int(y2)
                                }
                            }
                            detections.append(detection)

            return detections

        except Exception as e:
            print(f" YOLO inference hatası: {e}")
            return []

    def print_stats(self):
        """Basit istatistikler"""
        print(f"\n İstatistikler:")
        print(f"   İşlenen frame: {self.frame_count}")
        print(f"   Toplam tespit: {self.detection_count}")
        if self.frame_count > 0:
            print(f"   Ortalama tespit/frame: {self.detection_count/self.frame_count:.1f}")

if __name__ == "__main__":
    print(" YOLO Service")
    print("======================")

    service = SimpleYOLOService()

    try:
        service.start_server()
    except KeyboardInterrupt:
        print("\n️ Servis durduruldu")
    finally:
        service.print_stats()

