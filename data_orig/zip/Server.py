# NoCacheHTTPServer.py

import http.server

PORT = 80

class NoCacheHTTPRequestHandler(
    http.server.SimpleHTTPRequestHandler
):
    def send_response_only(self, code, message=None):
        super().send_response_only(code, message)
        self.send_header('Cache-Control', 'no-store, no-cache, must-revalidate, post-check=0, pre-check=0')
        self.send_header('Expires', '0')

if __name__ == '__main__':
    http.server.test(
        HandlerClass=NoCacheHTTPRequestHandler,
        port=PORT
    )