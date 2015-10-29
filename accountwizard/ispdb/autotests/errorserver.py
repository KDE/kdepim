# -*- coding: utf-8 -*-
# Copyright (c) 2015 Sandro Knauß <knauss@kolabsys.com>

# This library is free software; you can redistribute it and/or modify it
# under the terms of the GNU Library General Public License as published by
# the Free Software Foundation; either version 2 of the License, or (at your
# option) any later version.

# This library is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
# License for more details.

# You should have received a copy of the GNU Library General Public License
# along with this library; see the file COPYING.LIB.  If not, write to the
# Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
# 02110-1301, USA.


import BaseHTTPServer
import os.path

class ErrorServer(BaseHTTPServer.BaseHTTPRequestHandler):
    '''a simple server that always anwsers with the corresponding error code:
    /500 -> error 500
    '''
    def do_GET(self):
        self.send_error(int(os.path.basename(self.path)))

def run(server_class=BaseHTTPServer.HTTPServer,
        handler_class=ErrorServer):
    global keep_running
    server_address = ('localhost', 8000)
    httpd = server_class(server_address, handler_class)
    httpd.serve_forever()

run()
