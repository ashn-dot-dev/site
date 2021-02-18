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

## Server Setup
After adding your SSH public key to `~/.ssh/authorized_keys` update
`/etc/ssh/sshd_config` to contain the settings:
```txt
PermitRootLogin yes
PasswordAuthentication no
```
