import socket
import sys

class Client:
    def __init__(self, host='127.0.0.1', port=8080):
        self.host = host
        self.port = port
        self.socket = None

    def connect(self):
        try:
            self.socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            self.socket.connect((self.host, self.port))
            print(f"Connected to server {self.host}:{self.port}")
            return True
        except Exception as e:
            print(f"Connection failed: {e}")
            return False

    def disconnect(self):
        if self.socket:
            self.socket.close()
            self.socket = None

    def send_command(self, command):
        if not self.socket:
            return "ERROR: Not connected"

        try:
            self.socket.sendall(command.encode())

            response = self.socket.recv(100000).decode()
            return response
        except Exception as e:
            return f"ERROR: {e}"

def print_help():
    print("\nAvailable commands:")
    print("  search <term>           - Search for documents containing the term")
    print("  search_and <t1> <t2>... - Search for documents containing all terms")
    print("  search_or <t1> <t2>...  - Search for documents containing any term")
    print("  stats                   - Get index statistics")
    print("  freq <term>             - Get term frequency")
    print("  add_file <server_path>  - Add file to index (server-side)")
    print("  help                    - Show this help")
    print("  exit                    - Exit the client\n")

def main():
    host = '127.0.0.1'
    port = 8080

    if len(sys.argv) > 1:
        host = sys.argv[1]
    if len(sys.argv) > 2:
        port = int(sys.argv[2])

    client = Client(host, port)

    if not client.connect():
        return 1

    print_help()

    try:
        while True:
            try:
                line = input("> ").strip()
            except EOFError:
                break

            if not line:
                continue

            if line == "help":
                print_help()
                continue

            if line == "exit":
                client.send_command("EXIT")
                break

            if line.startswith("search"):
                if line.startswith("search_and"):
                    protocol_command = "SEARCH_AND" + line[10:]
                elif line.startswith("search_or"):
                    protocol_command = "SEARCH_OR" + line[9:]
                else:
                    protocol_command = "SEARCH" + line[6:]
            elif line.startswith("stats"):
                protocol_command = "STATS"
            elif line.startswith("freq"):
                protocol_command = "TERM_FREQ" + line[4:]
            elif line.startswith("add_file"):
                protocol_command = "ADD_FILE" + line[8:]
            else:
                print("Unknown command. Type 'help' for available commands.")
                continue

            response = client.send_command(protocol_command)
            print(response, end='')

    except KeyboardInterrupt:
        print("\nInterrupted")
    finally:
        client.disconnect()
        print("Finishing the work of the Python Client...")

    return 0

if __name__ == "__main__":
    sys.exit(main())