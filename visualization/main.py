import socket
import matplotlib.pyplot as plt
import numpy as np
from matplotlib.animation import FuncAnimation
import signal
import sys

HOST = 'localhost'
PORT = 8080
MAX_SAMPLES = 1000  # Максимальное количество хранимых значений

data_buffer = []
running = True 

def update(frame):
    global data_buffer
    if data_buffer:
        plt.cla() 
        plt.plot(data_buffer, label='Wave')
        plt.ylim(-2, 2) 
        plt.xlim(0, len(data_buffer)) 
        plt.xlabel('Samples')
        plt.ylabel('Amplitude')
        plt.title('Real-time Wave')
        plt.legend()
        plt.grid()

def signal_handler(sig, frame):
    global running
    running = False 

def main():
    global data_buffer, running

    # Для обработки Ctrl+C
    signal.signal(signal.SIGINT, signal_handler)

    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
        s.connect((HOST, PORT))
        
        plt.figure()
        ani = FuncAnimation(plt.gcf(), update, interval=10, save_count=1000, cache_frame_data=False)
        plt.show(block=False)

        while running: 
            data = s.recv(1024)
            if not data:
                break
            
            values = data.decode().strip().split('\n')
            for value in values:
                try:
                    parts = value.split(' + ')
                    if len(parts) == 2:
                        real_part = float(parts[0])
                        imag_part = float(parts[1].replace('i', '').strip())
                        data_buffer.append(real_part)
                except ValueError:
                    continue

            if len(data_buffer) > MAX_SAMPLES:
                data_buffer = data_buffer[-MAX_SAMPLES:] 

            plt.pause(0.0001)  

    plt.close()  

if __name__ == "__main__":
    main()


# import signal
# import socket
# import numpy as np
# import matplotlib.pyplot as plt
# from matplotlib.animation import FuncAnimation

# def signal_handler(sig, frame):
#    global running
#    running = False 

# signal.signal(signal.SIGINT, signal_handler)

# # Настройка параметров подключения
# HOST = '127.0.0.1'  # IP-адрес сервера
# PORT = 8080         # Порт сервера

# # Настройка сокета
# client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
# client_socket.connect((HOST, PORT))

# # Параметры для графика
# FFT_SIZE = 1024
# fig, ax = plt.subplots()
# x_data = np.arange(FFT_SIZE)
# y_data = np.zeros(FFT_SIZE)
# line, = ax.plot(x_data, y_data)

# # Функция для обновления графика
# def update_plot(frame):
#     global y_data
#     try:
#         # Чтение данных с сокета
#         data = client_socket.recv(8192).decode('utf-8')
        
#         # Преобразование данных из строки в комплексные значения
#         values = data.strip().split('\n')
#         y_data = np.array([complex(float(v.split()[0]), float(v.split()[1])) for v in values])
        
#         # Обновление данных графика
#         line.set_ydata(np.abs(y_data))  # Отображение амплитуды
#     except Exception as e:
#         print(f"Error: {e}")

#     return line,

# # Анимация графика с использованием FuncAnimation
# ani = FuncAnimation(fig, update_plot, interval=100)

# # Показать график
# plt.show()

# # Закрыть сокет после завершения
# client_socket.close()
