hdproductid: Display product info of hard drives on Linux
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
hdproductid is a command-line tool for Linux, written in C, which can
display the product (model) name, serial number and size of all drives
connected to a Linux system. For USB storage devices (e.g. pen drives), it
also displays the name of the manufacturer.

Run hdproductid without arguments to get a detailed listing of all connected
hard drives and USB storage devices. To get the info of only a few devices,
pass their names (e.g. /dev/sda) on the command-line.

hdproductid has been tested with Linux kernels 2.6.35 ... 4.4.0.

hdproductid uses the HDIO_GET_IDENTITY ioctl for non-USB devices, and
/sys/... (sysfs) for USB devices to get the info. hdproductid uses /sys...
to get a list of connected devices.

__END__
