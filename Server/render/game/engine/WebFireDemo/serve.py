from http.server import SimpleHTTPRequestHandler, ThreadingHTTPServer
from pathlib import Path
import argparse
import os


class DemoHandler(SimpleHTTPRequestHandler):
    def end_headers(self) -> None:
        self.send_header("Cross-Origin-Opener-Policy", "same-origin")
        self.send_header("Cross-Origin-Embedder-Policy", "require-corp")
        self.send_header("Cross-Origin-Resource-Policy", "same-origin")
        super().end_headers()


def main() -> None:
    parser = argparse.ArgumentParser(description="Serve the NewAOE fire demo.")
    parser.add_argument("directory", type=Path)
    parser.add_argument("--port", type=int, default=8125)
    args = parser.parse_args()

    directory = args.directory.resolve()
    os.chdir(directory)
    server = ThreadingHTTPServer(("127.0.0.1", args.port), DemoHandler)
    print(f"Serving {directory} at http://127.0.0.1:{args.port}/")
    server.serve_forever()


if __name__ == "__main__":
    main()
