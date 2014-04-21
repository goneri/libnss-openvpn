libnss-openvpn
==============

The libnss-openvpn name service switch module resolves the name
“xxx.vpn” to the associated xxx machine in the openvpn.server.status
file.


With this module, you can access to a given host from the openvpn server
even if the IP address is not static.

    $ ssh foobar.vpn

You don't need a ccd directory per host any more.

server.status file
==================

The module uses openvpn status file to get the informations. This file
expected location is /var/run/openvpn.server.status.

If the file is missing you can add the following line in your
server.conf file:

    # status /var/run/openvpn.server.status

If you want users to be able to use the module, you will probably
need to relax the security of openvpn.server.status:

    # chmod 644 /var/run/openvpn.server.status

Manual installation
===================

    $ make && make install

To make the system actually use this NSS module, add it to the list of
hosts modules in /etc/nsswitch.conf:

    hosts:          files dns openvpn

Credits
=======

libnss-openvpn is © 2012-2014 Gonéri Le Bouder and licensed under the terms of
the GNU General Public Licencse, version 2 or later (see LICENSE).

It incorporates some code from these modules:
   nss-mdns, © 2004 Lennart Poettering.
   nss-gw-name, © 2010 Joachim Breitner.
