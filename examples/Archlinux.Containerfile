FROM archlinux:latest

RUN pacman -Syu --noconfirm
RUN pacman -S --noconfirm base systemd
RUN pacman -S --noconfirm linux linux-firmware mkinitcpio

# set up gui
RUN pacman -S --noconfirm pantheon lightdm lightdm-pantheon-greeter
RUN systemctl enable lightdm

# common tools
RUN pacman -S --noconfirm bash zsh vim

# keyboard
# TODO

# locale
RUN echo "en_GB.UTF-8 UTF-8" >> /etc/locale.gen
RUN locale-gen
RUN echo "LANG=en_GB.UTF-8" > /etc/locale.conf

# timezone
RUN ln -sf /usr/share/zoneinfo/Europe/London /etc/localtime
RUN echo "Europe/London" > /etc/timezone

# set up network
RUN echo "nameserver 8.8.8.8" >> /etc/resolv.conf
RUN echo "myhost" > /etc/hostname
RUN pacman -S --noconfirm networkmanager
RUN systemctl enable NetworkManager

# set up user
RUN pacman -S --noconfirm sudo
RUN echo "%wheel ALL=(ALL) NOPASSWD: ALL" >> /etc/sudoers
RUN useradd -m -G wheel -s /bin/zsh baloot 
RUN echo "myuser:password" | chpasswd
