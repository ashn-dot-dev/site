New User Setup on a Debian/Ubuntu Server
========================================

This is my process to setup a new user with SSH access on a Debian/Ubuntu
server. In this example we will create a new user named `USER`. Before adding
the user, edit `/etc/ssh/sshd_config` as root and ensure the following
configuration settings are present:

```
PubkeyAuthentication yes
PasswordAuthentication no

# Add this line to the end of /etc/ssh/sshd_config if not already present.
# This line contains a list of users that may login via SSH - in this case,
# root, the user ashn, and our soon to be created user USER.
AllowUsers root ashn USER
```

Be sure to restart the SSH daemon with `systemctl restart sshd` for these
configuration settings to take effect.

## Setting Up a New User

Create the new user with `useradd`:

```sh
$ useradd -m -d /home/USER -s /bin/bash USER
```

Create the user's `.ssh` directory and `authorized_keys` file:

```sh
$ mkdir -p /home/USER/.ssh && touch /home/USER/.ssh/authorized_keys
```

Edit `authorized_keys` and paste the contents of the user's `id_rsa.pub` into
the file.

```sh
$ vim /home/USER/.ssh/authorized_keys
```

Set the permissions and ownership of the `.ssh` directory and `authorized_keys`
file in order to make the SSH daemon happy.

```sh
$ chmod 700 /home/USER/.ssh && chmod 600 /home/USER/.ssh/authorized_keys
$ chown -R USER:USER /home/USER/.ssh
```

## Deleting a User

To delete the user and their home directory run:

```sh
$ userdel -r USER
```
