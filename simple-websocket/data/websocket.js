"use strict";

var connection = new WebSocket( 'ws://' + location.hostname + ':81/', [ 'arduino' ] );
var LEDrougeLastValue;
var LEDbleueLastValue;

var ESP_MODULE_TYPE = 'ESP-12E'

if( ESP_MODULE_TYPE == 'ESP-01' ) {
  console.log( "on est sur un ESP-01" );
  var LEDrougeGPIO = 0;
  var LEDbleueGPIO = 2;
} else if( ESP_MODULE_TYPE == 'ESP-12E' ) {
  console.log( "on est sur un ESP-12E" );
  var LEDrougeGPIO = 16;
  var LEDbleueGPIO = 2;
}

document.addEventListener( "DOMContentLoaded", function( event ) {
    var btn0 = document.getElementById( "btn0" );
    var btn1 = document.getElementById( "btn1" );
    btn0.innerHTML = 'LED bleue (GPIO ' + LEDbleueGPIO + ')'
    btn1.innerHTML = 'LED rouge (GPIO ' + LEDrougeGPIO + ')'
});

connection.onopen = function() {
  console.log( 'Connexion établie' );
  connection.send( 'PAGE WEB - Connexion etablie : ' + new Date() );
};

connection.onerror = function( error ) {
  console.log( 'Erreur WebSocket ', error );
};

connection.onclose = function( error ) {
  console.log( 'Fermeture WebSocket ', error );
};

connection.onmessage = function( e ) {
  console.log( 'L’ESP8266 dit : ', e.data );
  console.log( 'length : ', e.data.length );
  var ESP8266rep = JSON.parse( e.data );

  if( "GPIO" in ESP8266rep ) {
    console.log( 'ESP8266rep.GPIO     = ', ESP8266rep.GPIO     );
    console.log( 'ESP8266rep.GPIO.G0  = ', ESP8266rep.GPIO.G0  );
    console.log( 'ESP8266rep.GPIO.G2  = ', ESP8266rep.GPIO.G2  );
    console.log( 'ESP8266rep.GPIO.G16 = ', ESP8266rep.GPIO.G16 );

    console.log( 'ESP8266rep.GPIO[ ' + LEDrougeGPIO + ' ] = ' + ESP8266rep.GPIO[ LEDrougeGPIO ] );
    console.log( 'ESP8266rep.GPIO[ ' + LEDbleueGPIO + ' ] = ' + ESP8266rep.GPIO[ LEDbleueGPIO ] );

    LEDrougeLastValue = ESP8266rep.GPIO[ LEDrougeGPIO ];
    LEDbleueLastValue = ESP8266rep.GPIO[ LEDbleueGPIO ];
  }
  else if( "TIME" in ESP8266rep ) {
    console.log( 'ESP8266rep.TIME = ', ESP8266rep.TIME );
    var times = ESP8266rep.TIME.split( " " );
    var pTime = document.getElementById( "horloge" );
    pTime.innerHTML = "Heure de démarrage de l’ESP<br/><br/>" + times[ 0 ] + "<br/>" + times[ 1 ];
  }

};

function envoyerTexte() {
  var txtLED = document.getElementById( "txtLED" ).value;
  console.log( 'PAGE WEB - Envoi de : ' + txtLED );
  connection.send( txtLED );
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
