#!/usr/bin/env python
# -*- coding: utf-8 -*-

##########
# websocket-client
# https://pypi.python.org/pypi/websocket-client/
# sudo -H pip install websocket-client
#####

from websocket import create_connection
ws = create_connection( "ws://192.168.1.132:81/python" )

msg = '#0000FF'
print "Envoi d’un message à l’ESP"
print( msg )
ws.send( msg )
print "Fin de l’envoi\n"

print "Réception..."
result = ws.recv()
print "Reçu : '%s'" % result
ws.close()
