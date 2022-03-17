#include "header.h"
#include "../config.h"
#include "../tcpsockets/tcp_socket.h"

#include <stdio.h>
#include <string.h>
#include <time.h>

#define BUF_SIZE 1024

void status(const int code, char *buf) {
  switch (code) {
  case STATUS_OK:
    strcpy(buf, "Ok");
    break;
  case STATUS_NOT_MODIFIED:
    strcpy(buf, "Not Modified");
    break;
  case STATUS_BAD_REQUEST:
    strcpy(buf, "Bad Request");
    break;
  case STATUS_FORBIDDEN:
    strcpy(buf, "Forbidden");
    break;
  case STATUS_NOT_FOUND:
    strcpy(buf, "Not Found");
    break;
  case STATUS_INTERNAL_SERVER_ERROR:
    strcpy(buf, "Internal Server Error");
    break;
  case STATUS_NOT_IMPLEMENTED:
    strcpy(buf, "Not Implemented");
    break;
  default:
    strcpy(buf, "Internal Server Error");
  }
}

void content_type(const char *ext, char *buf) {
  // Common MIME types from
  // https://developer.mozilla.org/en-US/docs/Web/HTTP/Basics_of_HTTP/MIME_types/Common_types
  if (strcmp("aac", ext) == 0) {
    strcpy(buf, "audio/aac");
  } else if (strcmp("abw", ext) == 0) {
    strcpy(buf, "application/x-abiword");
  } else if (strcmp("arc", ext) == 0) {
    strcpy(buf, "application/x-freearc");
  } else if (strcmp("avif", ext) == 0) {
    strcpy(buf, "image/avif");
  } else if (strcmp("avi", ext) == 0) {
    strcpy(buf, "video/x-msvideo");
  } else if (strcmp("azw", ext) == 0) {
    strcpy(buf, "application/vndamazonebook");
  } else if (strcmp("bin", ext) == 0) {
    strcpy(buf, "application/octet-stream");
  } else if (strcmp("bmp", ext) == 0) {
    strcpy(buf, "image/bmp");
  } else if (strcmp("bz", ext) == 0) {
    strcpy(buf, "application/x-bzip");
  } else if (strcmp("bz2", ext) == 0) {
    strcpy(buf, "application/x-bzip2");
  } else if (strcmp("cda", ext) == 0) {
    strcpy(buf, "application/x-cdf");
  } else if (strcmp("csh", ext) == 0) {
    strcpy(buf, "application/x-csh");
  } else if (strcmp("css", ext) == 0) {
    strcpy(buf, "text/css");
  } else if (strcmp("csv", ext) == 0) {
    strcpy(buf, "text/csv");
  } else if (strcmp("doc", ext) == 0) {
    strcpy(buf, "application/msword");
  } else if (strcmp("docx", ext) == 0) {
    strcpy(
        buf,
        "application/vndopenxmlformats-officedocumentwordprocessingmldocument");
  } else if (strcmp("eot", ext) == 0) {
    strcpy(buf, "application/vndms-fontobject");
  } else if (strcmp("epub", ext) == 0) {
    strcpy(buf, "application/epub+zip");
  } else if (strcmp("gz", ext) == 0) {
    strcpy(buf, "application/gzip");
  } else if (strcmp("gif", ext) == 0) {
    strcpy(buf, "image/gif");
  } else if (strcmp("htm", ext) == 0 || strcmp("html", ext) == 0) {
    strcpy(buf, "text/html");
  } else if (strcmp("ico", ext) == 0) {
    strcpy(buf, "image/vndmicrosofticon");
  } else if (strcmp("ics", ext) == 0) {
    strcpy(buf, "text/calendar");
  } else if (strcmp("jar", ext) == 0) {
    strcpy(buf, "application/java-archive");
  } else if (strcmp("jpeg", ext) == 0 || strcmp("jpg", ext) == 0) {
    strcpy(buf, "image/jpeg");
  } else if (strcmp("js", ext) == 0) {
    strcpy(buf, "text/javascript");
  } else if (strcmp("json", ext) == 0) {
    strcpy(buf, "application/json");
  } else if (strcmp("jsonld", ext) == 0) {
    strcpy(buf, "application/ld+json");
  } else if (strcmp("mid", ext) == 0 || strcmp("midi", ext) == 0) {
    strcpy(buf, "audio/midi");
  } else if (strcmp("mjs", ext) == 0) {
    strcpy(buf, "text/javascript");
  } else if (strcmp("mp3", ext) == 0) {
    strcpy(buf, "audio/mpeg");
  } else if (strcmp("mp4", ext) == 0) {
    strcpy(buf, "video/mp4");
  } else if (strcmp("mpeg", ext) == 0) {
    strcpy(buf, "video/mpeg");
  } else if (strcmp("mpkg", ext) == 0) {
    strcpy(buf, "application/vndappleinstaller+xml");
  } else if (strcmp("odp", ext) == 0) {
    strcpy(buf, "application/vndoasisopendocumentpresentation");
  } else if (strcmp("ods", ext) == 0) {
    strcpy(buf, "application/vndoasisopendocumentspreadsheet");
  } else if (strcmp("odt", ext) == 0) {
    strcpy(buf, "application/vndoasisopendocumenttext");
  } else if (strcmp("oga", ext) == 0) {
    strcpy(buf, "audio/ogg");
  } else if (strcmp("ogv", ext) == 0) {
    strcpy(buf, "video/ogg");
  } else if (strcmp("ogx", ext) == 0) {
    strcpy(buf, "application/ogg");
  } else if (strcmp("opus", ext) == 0) {
    strcpy(buf, "audio/opus");
  } else if (strcmp("otf", ext) == 0) {
    strcpy(buf, "font/otf");
  } else if (strcmp("png", ext) == 0) {
    strcpy(buf, "image/png");
  } else if (strcmp("pdf", ext) == 0) {
    strcpy(buf, "application/pdf");
  } else if (strcmp("php", ext) == 0) {
    strcpy(buf, "application/x-httpd-php");
  } else if (strcmp("ppt", ext) == 0) {
    strcpy(buf, "application/vndms-powerpoint");
  } else if (strcmp("pptx", ext) == 0) {
    strcpy(buf, "application/"
                "vndopenxmlformats-officedocumentpresentationmlpresentation");
  } else if (strcmp("rar", ext) == 0) {
    strcpy(buf, "application/vndrar");
  } else if (strcmp("rtf", ext) == 0) {
    strcpy(buf, "application/rtf");
  } else if (strcmp("sh", ext) == 0) {
    strcpy(buf, "application/x-sh");
  } else if (strcmp("svg", ext) == 0) {
    strcpy(buf, "image/svg+xml");
  } else if (strcmp("swf", ext) == 0) {
    strcpy(buf, "application/x-shockwave-flash");
  } else if (strcmp("tar", ext) == 0) {
    strcpy(buf, "application/x-tar");
  } else if (strcmp("tif tiff", ext) == 0) {
    strcpy(buf, "image/tiff");
  } else if (strcmp("ts", ext) == 0) {
    strcpy(buf, "video/mp2t");
  } else if (strcmp("ttf", ext) == 0) {
    strcpy(buf, "font/ttf");
  } else if (strcmp("txt", ext) == 0) {
    strcpy(buf, "text/plain");
  } else if (strcmp("vsd", ext) == 0) {
    strcpy(buf, "application/vndvisio");
  } else if (strcmp("wav", ext) == 0) {
    strcpy(buf, "audio/wav");
  } else if (strcmp("weba", ext) == 0) {
    strcpy(buf, "audio/webm");
  } else if (strcmp("webm", ext) == 0) {
    strcpy(buf, "video/webm");
  } else if (strcmp("webp", ext) == 0) {
    strcpy(buf, "image/webp");
  } else if (strcmp("woff", ext) == 0) {
    strcpy(buf, "font/woff");
  } else if (strcmp("woff2", ext) == 0) {
    strcpy(buf, "font/woff2");
  } else if (strcmp("xhtml", ext) == 0) {
    strcpy(buf, "application/xhtml+xml");
  } else if (strcmp("xls", ext) == 0) {
    strcpy(buf, "application/vndms-excel");
  } else if (strcmp("xlsx", ext) == 0) {
    strcpy(buf,
           "application/vndopenxmlformats-officedocumentspreadsheetmlsheet");
  } else if (strcmp("xml", ext) == 0) {
    strcpy(buf, "application/xml");
  } else if (strcmp("xul", ext) == 0) {
    strcpy(buf, "application/vndmozillaxul+xml");
  } else if (strcmp("zip", ext) == 0) {
    strcpy(buf, "application/zip");
  } else if (strcmp("3gp", ext) == 0) {
    strcpy(buf, "video/3gpp");
  } else if (strcmp("3g2", ext) == 0) {
    strcpy(buf, "video/3gpp2");
  } else if (strcmp("7z", ext) == 0) {
    strcpy(buf, "application/x-7z-compressed");
  } else {
    strcpy(buf, "text/plain");
  }
}

int header(tcp_socket *client, int scode, const char *ext, off_t fsize) {
  char status_str[128] = {0};
  char cont_type[128] = {0};
  char buf[BUF_SIZE] = {0};
  status(scode, status_str);
  content_type(ext, cont_type);
  char now[128] = {0};
  time_t t = time(NULL);
  struct tm *tm = localtime(&t);
  if (tm == NULL) {
    return -1;
  }
  strftime(now, 128, "%a, %d %b %Y %H:%M:%S GMT", tm);
  switch (scode) {
  case STATUS_OK:
    sprintf(buf,
            "HTTP/1.0 %d %s\r\n"
            "Content-Type: %s\r\n"
            "Content-Length: %zu\r\n"
            "Connection: keep-alive\r\n"
            "Date: %s\r\n"
            "Server: " SERVER_NAME "\r\n"
            "\r\n",
            scode, status_str, cont_type, fsize, now);
    break;
  case STATUS_NOT_MODIFIED:
    sprintf(buf,
            "HTTP/1.0 %d %s\r\n"
            "Date: %s\r\n"
            "Server: " SERVER_NAME "\r\n"
            "Content-type: text/html\r\n"
            "\r\n",
            scode, status_str, now);
    break;
  case STATUS_BAD_REQUEST:
  case STATUS_FORBIDDEN:
  case STATUS_NOT_FOUND:
  case STATUS_INTERNAL_SERVER_ERROR:
  case STATUS_NOT_IMPLEMENTED:
    sprintf(buf,
            "HTTP/1.0 %d %s\r\n"
            "Date: %s\r\n"
            "Server: " SERVER_NAME "\r\n"
            "Content-type: text/html\r\n"
            "Connection: close\r\n"
            "\r\n",
            scode, status_str, now);
    break;
  default:
    sprintf(buf,
            "HTTP/1.0 %d %s\r\n"
            "Date: %s\r\n"
            "Server: " SERVER_NAME "\r\n"
            "Content-type: text/html\r\n"
            "Connection: close\r\n"
            "\r\n",
            STATUS_INTERNAL_SERVER_ERROR, status_str, now);
  }
  if (write_socket_tcp(client, buf, strlen(buf)) == -1) {
    fprintf(stderr, "Couldn't send response\n");
    return -1;
  }
  return 0;
}
