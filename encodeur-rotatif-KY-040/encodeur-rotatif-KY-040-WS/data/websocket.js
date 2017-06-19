"use strict";

var connection;
openWebSocket();

function openWebSocket() {
  try {
    connection = new WebSocket( 'ws://' + location.hostname + ':81/', [ 'arduino' ] );
  } catch( exception ) {
    console.error( exception );
  }
}

setInterval( getConnectionState, 1000 );
function getConnectionState() {
  var state = connection.readyState;
  if( state === 1 ) // OPEN
    return;
  else if( state === 0 ) // CONNECTING
    console.log( "# LA CONNEXION EST EN COURS D’OUVERTURE" );
  else if( state === 2 ) // CLOSING
    console.log( "# LA CONNEXION EST EN COURS DE FERMETURE" );
  else if( state === 3 ) // CLOSED
    console.log( "# LA CONNEXION EST FERMÉE" );
  else
    console.log( "# LA CONNEXION EST DANS UN ÉTAT INCONNU" );
}

var LEDrougeLastValue;
var LEDbleueLastValue;

var ESP_MODULE_TYPE = 'ESP-12E'

if( ESP_MODULE_TYPE == 'ESP-01' ) {
  console.log( "on est sur un ESP-01" );
  var LEDrougeGPIO = "GPIO0";
  var LEDbleueGPIO = "GPIO2";
}
else if( ESP_MODULE_TYPE == 'ESP-12E' ) {
  console.log( "on est sur un ESP-12E" );
  var LEDrougeGPIO = "GPIO16";
  var LEDbleueGPIO = "GPIO2";
}

document.addEventListener( "DOMContentLoaded", function( event ) {
    var btn0 = document.getElementById( "btn0" );
    var btn1 = document.getElementById( "btn1" );
    btn0.innerHTML = 'LED bleue (GPIO ' + LEDbleueGPIO + ')';
    btn1.innerHTML = 'LED rouge (GPIO ' + LEDrougeGPIO + ')';
});

connection.onopen = function() {
  console.log( 'Connexion établie' );
  connection.send( 'PAGE WEB - Connexion etablie : ' + new Date() );
  demanderIP();
};

connection.onerror = function( error ) {
  console.log( 'Erreur WebSocket ', error );
};

connection.onclose = function( error ) {
  console.log( 'Fermeture WebSocket ', error );
};

connection.onmessage = function( e ) {
  console.log( 'COUCOU L’ESP8266 dit : ', e.data );
  console.log( 'length : ', e.data.length );
  var ESP8266rep = JSON.parse( e.data );

  for( var elem in ESP8266rep ) {
    console.log( 'elem = ' + elem  );
         if( elem === 'cpt'      ) receivedCPT( ESP8266rep );
    else if( elem === 'TIME'     ) receivedTIME( ESP8266rep );
    else if( elem === 'IP'       ) receivedIP( ESP8266rep );
    else if( elem === 'mDNSName' ) receivedmDNSName( ESP8266rep );
    else if( elem === 'GPIO'     ) receivedGPIO( ESP8266rep );
  }
};

function receivedCPT( ESP8266rep ) {
  console.log( 'ESP8266rep.cpt     = ', ESP8266rep.cpt.cpt1 );
  document.getElementById( "txtLED" ).value = ESP8266rep.cpt.cpt1;
  document.getElementById( "cpt-progress" ).value = ESP8266rep.cpt.cpt1;
}

function receivedGPIO( ESP8266rep ) {
  console.log( 'ESP8266rep.GPIO     = ', ESP8266rep.GPIO     );
  console.log( 'ESP8266rep.GPIO[ ' + LEDrougeGPIO + ' ] = ' + ESP8266rep.GPIO[ LEDrougeGPIO ] );
  console.log( 'ESP8266rep.GPIO[ ' + LEDbleueGPIO + ' ] = ' + ESP8266rep.GPIO[ LEDbleueGPIO ] );
  LEDrougeLastValue = ESP8266rep.GPIO[ LEDrougeGPIO ];
  LEDbleueLastValue = ESP8266rep.GPIO[ LEDbleueGPIO ];
}

function receivedTIME( ESP8266rep ) {
  console.log( 'ESP8266rep.TIME = ', ESP8266rep.TIME );
  var times = ESP8266rep.TIME.split( " " );
  var pTime = document.getElementById( "horloge" );
  pTime.innerHTML = "Heure de démarrage de l’ESP<br/><br/>" + times[ 0 ] + "<br/>" + times[ 1 ];
}

function receivedIP( ESP8266rep ) {
  var adresseIP = document.getElementById( "adresseIP" );
  adresseIP.innerHTML = '<a href="http://' + ESP8266rep.IP + '">' + ESP8266rep.IP + '</a>';
}

function receivedmDNSName( ESP8266rep ) {
  var mDNSName = document.getElementById( "mDNSName" );
  mDNSName.innerHTML = '<a href="http://' + ESP8266rep.mDNSName + '.local">' + ESP8266rep.mDNSName + '</a>';
}

function envoyerTexte() {
  var txtLED = document.getElementById( "txtLED" ).value;
  console.log( 'PAGE WEB - Envoi de : ' + txtLED );
  connection.send( txtLED );
}

function demanderIP() {
  console.log( 'PAGE WEB - Demande adresse IP' );
  connection.send( 'WhatIsYourIP' );
}

function changeLEDrouge() {
  if( LEDrougeLastValue == 0 ) {
    addClass( btn1, "pressed" );
    var rgb = '#FF0000';
    LEDrougeLastValue = 1;
  } else {
    removeAllClasses( btn1 );
    var rgb = '#000000';
    LEDrougeLastValue = 0;
  }
  console.log( 'PAGE WEB - RGB: ' + rgb );
  connection.send( rgb );
}

function changeLEDbleue() {
  var btn1 = document.getElementById( "btn1" );
  if( LEDbleueLastValue == 0 ) {
    addClass( btn0, "pressed" );
    var rgb = '#0000FF';
    LEDbleueLastValue = 1;
  } else {
    removeAllClasses( btn0 );
    var rgb = '#000000';
    LEDbleueLastValue = 0;
  }
  console.log( 'PAGE WEB - RGB: ' + rgb );
  connection.send( rgb );
}

function sleep( milliseconds ) {
  var start = new Date().getTime();
  for( var i = 0; i<1e7; i++ ) {
    if( ( new Date().getTime() - start ) > milliseconds )
      { break; }
  }
}

function removeAllClasses( elem ) {
  elem.className = "";
}

function addClass( elem, clazz ) {
  if( !elemHasClass( elem, clazz ) ) {
    elem.className += " " + clazz;
  }
}

function elemHasClass( elem, clazz ) {
  return new RegExp( "( |^)" + clazz + "( |$)" ).test( elem.className );
}

