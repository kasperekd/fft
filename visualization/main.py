import socket
import matplotlib.pyplot as plt
import numpy as np
from matplotlib.animation import FuncAnimation

host = 'localhost'
port = 8080
data_buffer = []

def update(frame):
    global data_buffer
    if data_buffer:
        plt.cla() 
        plt.plot(data_buffer, label='Sine Wave')
        plt.ylim(-1.5, 1.5) 
        plt.xlim(0, len(data_buffer)) 
        plt.xlabel('Samples')
        plt.ylabel('Amplitude')
        plt.title('Real-time Sine Wave')
        plt.legend()
        plt.grid()

def main():
    global data_buffer

    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
        s.connect((host, port))
        
        plt.figure()
        
        ani = FuncAnimation(plt.gcf(), update, interval=100)  # Обновление каждые 100 мс

        while True:
            data = s.recv(1024)
            if not data:
                break
            
            values = data.decode().strip().split('\n')
            for value in values:
                try:
                    float_value = float(value)
                    data_buffer.append(float_value)
                except ValueError:
                    continue

            if len(data_buffer) > 100:  # Храним последние 100 значений
                data_buffer.pop(0)

            plt.pause(0.01)

    plt.show()

if __name__ == "__main__":
    main()
