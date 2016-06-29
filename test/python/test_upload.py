# To run the test: py.test -s -v test_upload.py

import cherrypy, time, httplib, os, sys
from cherrypy.test import helper


class UploadTest(helper.CPWebCase):
  def setup_server():
    class Root:
      @cherrypy.expose
      def put(self, size, xtra, file):
        assert xtra == 'foo'
        now = time.time()
        filelen = 0
        while True:
          data = file.file.read(8*1024*1024)
          if not data: break
          filelen += len(data)
        cherrypy.response.headers["X-Timing"] = "%.3f" % (time.time() - now)
        return "thanks len %d orig %d file '%s'" % (int(size), filelen, file.filename)

    bufsize = int(os.environ.get("BODY_IO_SIZE", 1<<19))
    cherrypy.tree.mount(Root(), "/small", config={'/': {}})
    cherrypy.tree.mount(Root(), "/big", config={'/': {'request.body_io_size': bufsize}})
    cherrypy.config.update({'server.max_request_body_size': 0, 'environment': 'test_suite', 'server.port': 8080})
  setup_server = staticmethod(setup_server)

  def _encode(self, args, files, with_length):
    body, crlf = '', '\r\n'
    boundary = 'TEST'
    for key, value in args.iteritems():
      body += '--' + boundary + crlf
      body += ('Content-Disposition: form-data; name="%s"' % key) + crlf
      body += crlf + str(value) + crlf
    for key, file in files.iteritems():
      filename, filedata = file
      body += '--' + boundary + crlf
      body += ('Content-Disposition: form-data; name="%s"; filename="%s"'
               % (key, filename)) + crlf
      body += ('Content-Type: application/octet-stream') + crlf
      if with_length:
        body += ('Content-Length: %d' % len(filedata)) + crlf
      body += crlf + filedata + crlf
    body += '--' + boundary + '--' + crlf + crlf
    return 'multipart/form-data; boundary=' + boundary, body

  def _marshall(self, name, kbytes, with_length):
    data = ("x" * 127 + "\n") * 8 * kbytes
    args = { 'size': len(data), 'xtra': 'foo' }
    files = { 'file': (name, data) }
    type, body = self._encode(args, files, with_length)
    headers = [('Content-Type', type), ('Content-Length', str(len(body)))]
    return headers, body

  def _run_test(self, kind, kbytes, with_length):
    nbytes = kbytes*1024
    headers, body = self._marshall(kind + 'file', kbytes, with_length)
    start = time.time()
    self.getPage("/%s/put" % kind, method='POST', headers = headers, body = body)
    self.assertStatus(200)
    self.assertBody("thanks len %d orig %d file '%sfile'" % (nbytes, nbytes, kind))
    self.assertHeader("X-Timing")
    xtiming = [v for k, v in self.headers if k.lower() == "x-timing"][0]
    print "\nkind: %s,with_length: %s\t time(client=%.3f server=%s)" % (kind, str(with_length), time.time() - start, xtiming),

  def test_small_smart(self): self._run_test('small', 10, True)
  def test_small_dumb(self):  self._run_test('small', 10, False)
  def test_big_smart(self):   self._run_test('big', 150*1024, True)
  def test_big_dumb(self):    self._run_test('big', 150*1024, False)
