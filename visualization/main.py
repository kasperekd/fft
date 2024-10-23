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

    # для Ctrl+C
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
                    float_value = float(value)
                    data_buffer.append(float_value)
                except ValueError:
                    continue

            if len(data_buffer) > MAX_SAMPLES:
                data_buffer = data_buffer[-MAX_SAMPLES:] 

            plt.pause(0.01)  

    plt.close()  

if __name__ == "__main__":
    main()
