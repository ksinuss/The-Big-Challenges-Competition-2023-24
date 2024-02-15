# подключение библиотек
import os
import cv2
import time
from serial import Serial
from http.server import BaseHTTPRequestHandler, HTTPServer

# инициализация переменных
flag = 0
photo = 1
data = ''
gruz = ''
gruzy = {
    'a': 'винт',
    'b': 'гайка',
    'c': 'шайба',
    'd': 'шпилька',
    'e': 'подшипнк',
    'f': 'направл',
    'g': 'вал',
    'h': 'двигатель',
    'i': 'датчик'
}
bgruzy = {
    'a': b'VINT ',
    'b': b'GAIKA ',
    'c': b'SHAIBA ',
    'd': b'SHPILKA ',
    'e': b'PODSHIPNIK ',
    'f': b'NAPRAVLYAUSHAYA ',
    'g': b'VAL ',
    'h': b'DVIGATEL ',
    'i': b'DATCHIK '
}
zakaz = {}
finish = 0
take = ''
option = []
line = ''

# инициализация последовательного интерфейса
path1 = '/dev/ttyACM1'
path2 = '/dev/ttyACM0'
ser = Serial(path1, 9600, timeout=1)
ser.flush()
serr = Serial(path2, 9600, timeout=1)
serr.flush()

# функция декодирования qr-кода
def decod(cap, detector):
    _, img = cap.read()
    data, bbox, _ = detector.detectAndDecode(img)
    if(bbox is not None):
        for i in range(len(bbox)):
            cv2.line(img, tuple(bbox[i][0]), tuple(bbox[(i+1) % len(bbox)][0]), color=(255,
                     0, 255), thickness=2)
        cv2.putText(img, data, (int(bbox[0][0][0]), int(bbox[0][0][1]) - 10), cv2.FONT_HERSHEY_SIMPLEX,
                    0.5, (0, 255, 0), 2)
        if data: return data
        else: return ''

# распознавание
def get_data(zakaz):
    global path1, path2, flag, take, bgruzy, photo
    ser = Serial(path1, 9600, timeout=1)
    ser.flush()
    serr = Serial(path2, 9600, timeout=1)
    serr.flush()
    # получаем изображение с камеры
    cap = cv2.VideoCapture(0)
    detector = cv2.QRCodeDetector()
    # декодируем и передаем информацию о распознанном грузе
    data = decod(cap, detector)
    # проверяем на наличие груза в заказе
    if photo:
        if data in zakaz and data != take:
            photo = 0
            gruz = zakaz[data]
            ser.write(bgruzy[gruz])
            serr.write(b'1')
            take = data
            print(gruz)
            return gruz
        else:
            return ''
    else:
        if data == '1':
            photo = 1
        return 'w'

# взаимодействие с мобильным приложением
class ServerHandler(BaseHTTPRequestHandler):
    def do_GET(self):
        global gruzy, zakaz, path1, flag, ser, path1
        ser = Serial(path1, 9600, timeout=1)
        ser.flush()
        message_to_send = ''
        print("GET request, path:", self.path)
        Request = self.path
        Request = Request[1:]
        print(Request)
        # формирование заказа
        if 2 <= len(Request) <= 3:
            zakaz = {}
            for request in Request:
                if request in gruzy.keys():
                    name = gruzy[request]
                    zakaz[name] = request
        if Request == '1' and zakaz:
            # старт механической подсистемы
            flag = 1
            if len(zakaz) == 2:
                ser.write(b'2')
            else:
                ser.write(b'3')
        elif Request == '0':
            ser.write(b'0')
            flag = 0
        elif flag == 1:
            message_to_send = get_data(zakaz)
        print(message_to_send)
        # send from rasp to server
        bytes_to_send = bytes(message_to_send, "utf")
        self.send_response(200)
        self.send_header('Content-Type', 'text/plain')
        self.send_header('Content-Length', len(bytes_to_send))
        self.end_headers()
        self.wfile.write(bytes_to_send)
        return

# открываем сервер
server_address = ('192.168.100.125', 8080)
httpd = HTTPServer(server_address, ServerHandler)
print('Starting server:')
httpd.serve_forever()
