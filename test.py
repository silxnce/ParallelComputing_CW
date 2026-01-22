from locust import User, task, between, events
import socket
import time
import random

SERVER_HOST = "127.0.0.1"
SERVER_PORT = 8080
BUFFER_SIZE = 4096

TERMS = [
    "movie", "film", "good", "bad", "story",
    "love", "war", "life", "time", "man"
]

class TcpClient:
    def __init__(self):
        self.sock = None

    def connect(self):
        self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.sock.connect((SERVER_HOST, SERVER_PORT))

    def close(self):
        if self.sock:
            self.sock.close()
            self.sock = None

    def send_command(self, command: str):
        self.sock.sendall(command.encode())
        return self.sock.recv(BUFFER_SIZE).decode()


class InvertedIndexUser(User):
    wait_time = between(0.1, 0.5)

    def on_start(self):
        self.client = TcpClient()
        self.client.connect()

    def on_stop(self):
        self.client.close()

    def _measure(self, name, command):
        start = time.perf_counter()
        try:
            response = self.client.send_command(command)
            total_time = (time.perf_counter() - start) * 1000

            events.request.fire(
                request_type="TCP",
                name=name,
                response_time=total_time,
                response_length=len(response),
                exception=None
            )
        except Exception as e:
            total_time = (time.perf_counter() - start) * 1000
            events.request.fire(
                request_type="TCP",
                name=name,
                response_time=total_time,
                response_length=0,
                exception=e
            )

    @task(1)
    def search(self):
        term = random.choice(TERMS)
        self._measure("SEARCH", f"SEARCH {term}")

    @task(1)
    def search_and(self):
        t1, t2 = random.sample(TERMS, 2)
        self._measure("SEARCH_AND", f"SEARCH_AND {t1} {t2}")

    @task(1)
    def search_or(self):
        t1, t2 = random.sample(TERMS, 2)
        self._measure("SEARCH_OR", f"SEARCH_OR {t1} {t2}")

    @task(1)
    def stats(self):
        self._measure("STATS", "STATS")

    @task(1)
    def term_freq(self):
        term = random.choice(TERMS)
        self._measure("TERM_FREQ", f"TERM_FREQ {term}")