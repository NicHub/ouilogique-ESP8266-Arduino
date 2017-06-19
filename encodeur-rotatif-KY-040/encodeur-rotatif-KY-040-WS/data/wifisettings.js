'use strict';


if( document.domain.length === 0 ) {
    var url1 = "http://esp8266.local/wifisettings.json";
    var url2 = "http://esp8266.local/wifisettingsactiveindex.ini";
}
else {
    var url1 = "wifisettings.json";
    var url2 = "wifisettingsactiveindex.ini";
}

var xhr1 = new XMLHttpRequest();
xhr1.open( 'GET', url1 );
xhr1.send();
xhr1.addEventListener( "load", fill_wifi_settings_list );
xhr1.addEventListener( "error", function(){ console.log( "erreur" ) } );

function fill_wifi_settings_list() {
    if( this.status !== 200 ) {
        console.log( "erreur status = " + this.status );
        return;
    }
    var wifiSettings = JSON.parse( this.responseText );
    var nbElem = wifiSettings.length;
    var wifi_list = document.getElementById( "wifi_list" );
    var wifi_item_1 = document.getElementById( "wifi_item_1" );
    wifi_list.removeChild( wifi_item_1 );

    for( var i=1; i<=nbElem; i++ ) {
        var wifi_next_item =  wifi_item_1.cloneNode( true );
        wifi_next_item.id = "wifi_item_" + i ;

        var wifi_item_id = wifi_next_item.querySelector( ".wifi_item_id" );
        wifi_item_id.textContent = i;

        var wifi_item_ssid = wifi_next_item.querySelector( ".wifi_item_ssid" );
        wifi_item_ssid.value = wifiSettings[ i-1 ].SSID;

        var mdp = wifi_next_item.querySelector( ".wifi_item_mdp" );
        mdp.value = wifiSettings[ i-1 ].PASSWORD;
        // mdp.type = "text";

        wifi_list.appendChild( wifi_next_item );
    };

    getWifiSettingsActiveIndex()
}


function getWifiSettingsActiveIndex() {
    var xhr2 = new XMLHttpRequest();
    xhr2.open( 'GET', url2 );
    xhr2.send();
    xhr2.addEventListener( "load", checkActiveWifiIndex );
    xhr2.addEventListener( "error", function(){ console.log( "erreur" ) } );
}

function checkActiveWifiIndex() {
    if( this.status !== 200 ) {
        console.log( "erreur status = " + this.status );
        return;
    }
    var currentWifiID = 1 + parseInt( this.responseText, 10 );
    var currentWifi = document.querySelector( '#wifi_item_' + currentWifiID + ' input[type="radio"]' );
    currentWifi.checked = true;
}

