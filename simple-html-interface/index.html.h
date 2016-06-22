R"=======(<!DOCTYPE html>
<html>
<head>
<meta charset="utf-8" />
<meta http-equiv="X-UA-Compatible" content="IE=edge" />
<meta name="viewport" content="width=device-width, initial-scale=1" />
<title>⊛ ESP8266 ⊛</title>
<style>
html {
  color: #fff;
  font-family: "Helvetica Neue", sans-serif;
  text-align: center;
    min-height: 100%;
  /* Source : http://uigradients.com/# */
  background: #182848;
  background: -webkit-linear-gradient( to bottom, #4b6cb7 , #182848 );
  background: linear-gradient( to bottom, #4b6cb7 , #182848 );
}
body * {
  background-color: transparent;
  line-height: 120%;
  letter-spacing: 0.1em;
  word-spacing: -0.1em;
}
div {
  margin: 15px auto;
  display: table;
  max-width: 600px;
  padding: 10px;
}
h1 {
  font-size: 1.825em;
  margin: 15px auto;
  letter-spacing: 0.15em;
  word-spacing: -0.15em;
}
img {
  margin: 30px auto;
}
label {
  display: block;
  font-size: 16pt;
  font-weight: bold;
  margin: 0 auto;
  width: 100%;
}
textarea {
  border-color: #CCC;
  border-radius: 6px;
  border-style: solid;
  border-width: 1px;
  box-sizing: border-box;
  color: lime;
  font-family: Courier, monospace;
  font-size: 20pt;
  height: 250px;
  margin: 10px auto 15px;
  padding: 12px;
  width: 100%;
}
button {
  -webkit-appearance: none;
  -webkit-user-select: none;
  background-color: #337ab7;
  border-radius: 4px;
  border: 1px solid #2e6da4;
  color: white;
  font-size: 15pt;
  margin: 0 auto;
  min-height: 50px;
  padding: 10px;
  vertical-align: middle;
  width: 100%;
}
button:hover {
  cursor: pointer;
}
div.alert {
  color: #999;
  font-size: 14pt;
  text-align: center;
}
div.bad {
  color: red;
}
.neon {
  text-transform: uppercase;
}
.neon:hover {
  text-shadow: 0 0 5px #fff, 0 0 10px #fff, 0 0 15px #fff, 0 0 20px #ff00de, 0 0 30px #ff00de, 0 0 40px #ff00de, 0 0 50px #ff00de, 0 0 75px #ff00de;
}
h1.neon {
  margin-bottom: 100px;
}
div#label {
  max-width: 240px;
}
</style>
<script>

var connection = new WebSocket('ws://'+location.hostname+':81/', ['arduino']);
var LEDrougeLastValue;
var LEDbleueLastValue;

connection.onopen = function(){
  console.log( 'Connexion établie ' );
  connection.send( 'PAGE WEB - Connexion etablie : ' + new Date() );
};

connection.onerror = function( error ){
  console.log( 'Erreur WebSocket ', error );
};

connection.onmessage = function( e ){
  console.log( 'L’ESP8266 dit : ', e.data );
  var GPIO = JSON.parse( e.data );
  console.log( 'GPIO.GPIO02  = ', GPIO.GPIO2 );
  console.log( 'GPIO.GPIO16 = ', GPIO.GPIO16 );
  LEDrougeLastValue = GPIO.GPI16;
  LEDbleueLastValue = GPIO.GPIO2;
};

function sendRGB() {
  var r = parseInt(document.getElementById('r').value).toString(16);
  var g = parseInt(document.getElementById('g').value).toString(16);
  var b = parseInt(document.getElementById('b').value).toString(16);
  if(r.length < 2) { r = '0' + r; }
  if(g.length < 2) { g = '0' + g; }
  if(b.length < 2) { b = '0' + b; }
  var rgb = '#'+r+g+b;
  console.log('RGB: ' + rgb);
  connection.send(rgb);
}

function sendTextToRS232()
{
  var texte = document.getElementById( "txtLED" ).value;
  console.log( 'texte: ' + texte );
  connection.send( texte );
}


function changeLEDrouge()
{
  if( LEDrougeLastValue == 0 )
  {
    var rgb = '#FF0000';
    LEDrougeLastValue = 1;
  }
  else
  {
    var rgb = '#000000';
    LEDrougeLastValue = 0;
  }
  console.log('RGB: ' + rgb);
  connection.send(rgb);
}

function changeLEDbleue()
{
  if( LEDbleueLastValue == 0 )
  {
    var rgb = '#0000FF';
    LEDbleueLastValue = 1;
  }
  else
  {
    var rgb = '#000000';
    LEDbleueLastValue = 0;
  }
  console.log('RGB: ' + rgb);
  connection.send(rgb);
}


</script>
</head>
<body>

<div class="container">
<h1 class="neon">Test WebSocket<br />sur ESP8266</h1>
<label>Texte à envoyer via RS232</label>
<textarea id="txtLED"></textarea>
<button type="button" onclick="sendTextToRS232()">Envoyer le&nbsp;texte</button>
<div id="label" class="alert">❧</div>
</div>


<div class="container">
<button type="button" onclick="changeLEDbleue()">LED Bleue (GPIO 02)</button>
<br/><br/>
<button type="button" onclick="changeLEDrouge()">LED Rouge (GPIO 16)</button>
</div>


<h1>LED Control</h1>
<br/>R: <input id="r" type="range" min="0" max="255" step="1" onchange="sendRGB();" />
<br/>G: <input id="g" type="range" min="0" max="255" step="1" onchange="sendRGB();" />
<br/>B: <input id="b" type="range" min="0" max="255" step="1" onchange="sendRGB();" />
</body>
</html>


)=======";