# This Python file uses the following encoding: utf-8


import socket
import json
import struct
import base64
import cv2
import numpy as np
from ultralytics import YOLO
import time
import os

class CustomDroneYOLOService:
    def __init__(self):
        print("Custom Drone YOLO Service başlatılıyor...")

        # Custom drone modelini yükle
        self.model_path = self.find_drone_model()

        try:
            self.model = YOLO(self.model_path)
            print(f"Custom drone model yüklendi: {self.model_path}")

            # Model info
            print(f"Model classes: {self.model.names}")
            print(f"odel type: {self.model.task}")

        except Exception as e:
            print(f"Model yükleme hatası: {e}")
            print("Fallback: Genel model kullanılacak Drone detection geçersiz.")
            self.model = YOLO('yolo11n.pt')
            self.model_path = 'yolo11n.pt'

        # Server ayarları
        self.host = 'localhost'
        self.port = 8888
        self.socket = None

        # Drone detection için optimize edilmiş parametreler
        self.confidence_threshold = 0.40
        self.iou_threshold = 0.4
        self.imgsz = 640

        # İstatistikler
        self.frame_count = 0
        self.detection_count = 0
        self.total_processing_time = 0.0

    def start_server(self):
        """TCP server başlat ve C++'dan bağlantı bekle"""
        try:
            self.socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            self.socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
            self.socket.bind((self.host, self.port))
            self.socket.listen(1)

            print(f"  Server başlatıldı: {self.host}:{self.port}")
            print("  C++ bağlantısı bekleniyor...")

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
                print("  Server kapatıldı")


    def process_frames(self, client_socket):
        """C++'dan gelen frame'leri işle"""
        print(" Frame işleme başladı...")

        try:
            while True:
                # C++ dan mesaj al
                message = self.receive_message(client_socket)
                if not message:
                    break

                # Frame i işle
                if message.get('type') == 'frame_request':
                    self.handle_frame(client_socket, message)

        except KeyboardInterrupt:
            print("\nKullanıcı tarafından durduruldu")
        except Exception as e:
            print(f"Frame işleme hatası: {e}")
        finally:
            client_socket.close()
            print(" C++ bağlantısı kapatıldı")



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

    def find_drone_model(self):
        """Custom drone modelini bul"""
        possible_paths = [
            'best.pt',
            'drone_model.pt'      # Custom name
        ]

        for path in possible_paths:
            if os.path.exists(path):
                print(f" Drone model bulundu: {path}")
                return path

        print("Custom drone model bulunamadı, genel model kullanılacak")
        return 'yolo11n.pt'

    def detect_objects(self, frame, frame_id):
        """Custom drone detection with optimized parameters"""
        if self.model is None:
            print(" Model yüklü değil")
            return []

        try:
            start_time = time.time()

            # Custom drone detection parametreleri
            results = self.model(
                frame,
                conf=self.confidence_threshold,
                iou=self.iou_threshold,
                imgsz=self.imgsz,
                verbose=False,
                augment=True,
                agnostic_nms=True,
                half=False
            )

            processing_time = (time.time() - start_time) * 1000
            self.total_processing_time += processing_time

            detections = []

            for r in results:
                boxes = r.boxes
                if boxes is not None:
                    for box in boxes:
                        class_id = int(box.cls[0])
                        confidence = float(box.conf[0])

                        # Class name al
                        if class_id < len(self.model.names):
                            class_name = self.model.names[class_id]
                        else:
                            class_name = f"class_{class_id}"

                        # Bounding box koordinatları
                        x1, y1, x2, y2 = box.xyxy[0].tolist()

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

            # Debug info
            if detections:
                print(f"Frame {frame_id}: {len(detections)} drone bulundu")
                for det in detections:
                    print(f" {det['class_name']}: {det['confidence']:.3f}")

            return detections

        except Exception as e:
            print(f"Drone detection hatası: {e}")
            return []

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


    def handle_frame(self, client_socket, message):
        """Frame'i custom drone model ile işle"""
        try:
            payload = message['payload']
            frame_id = payload['frame_id']

            print(f"Drone detection - Frame: {frame_id}")

            # Base64'dan OpenCV frame'e çevir
            frame = self.decode_frame(payload['data'])
            if frame is None:
                print("Frame decode edilemedi")
                return

            # Custom drone detection
            detections = self.detect_objects(frame, frame_id)

            # Processing time hesapla
            avg_time = self.total_processing_time / max(1, self.frame_count)

            # Sonucu C++a gönder
            response = {
                'type': 'detection_result',
                'payload': {
                    'frame_id': frame_id,
                    'detections': detections,
                    'processing_time_ms': avg_time,
                    'model_info': {
                        'model_path': self.model_path,
                        'confidence_threshold': self.confidence_threshold,
                        'total_detections': len(detections)
                    }
                }
            }

            self.send_message(client_socket, response)

            # İstatistikleri güncelle
            self.frame_count += 1
            self.detection_count += len(detections)

            if self.frame_count % 50 == 0:  # Her 50 frame de istatistik
                print(f" İstatistik: {self.frame_count} frame, {self.detection_count} detection, avg: {avg_time:.1f}ms")

        except Exception as e:
            print(f"Frame işleme hatası: {e}")

    def print_stats(self):
        """Gelişmiş istatistikler"""
        print(f"\n CUSTOM DRONE DETECTION İSTATİSTİKLERİ")
        print("=" * 50)
        print(f"   Model: {self.model_path}")
        print(f"   İşlenen frame: {self.frame_count}")
        print(f"   Toplam drone tespiti: {self.detection_count}")

        if self.frame_count > 0:
            detection_rate = (self.detection_count / self.frame_count) * 100
            avg_processing = self.total_processing_time / self.frame_count
            print(f"   Detection rate: {detection_rate:.1f}%")
            print(f"   Ortalama işleme süresi: {avg_processing:.1f}ms")
            print(f"   Confidence threshold: {self.confidence_threshold}")


if __name__ == "__main__":
    print("CUSTOM DRONE YOLO SERVICE")
    print("=" * 40)

    service = CustomDroneYOLOService()

    try:
        service.start_server()
    except KeyboardInterrupt:
        print("\n,Servis durduruldu")
    finally:
        service.print_stats()
