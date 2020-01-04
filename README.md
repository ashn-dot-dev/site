ASHN'S PERSONAL WEBSITE
=======================

## Deploy Locally Over HTTP
```sh
$ ./build.bash && (cd out/ && python3 -m http.server)
```

## Deploy to Server
```sh
$ ./build.bash
$ scp -r out/* USER@SERVER:/var/www/html/
```
