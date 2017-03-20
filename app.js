var ttn = require('ttn');
var fs = require('fs');
var mysql = require('mysql');


var region = 'eu';
var appId = 'erwin50';
var accessKey = 'ttn-account-v2.oM9KJ_5rlECVTGUofIAEMbGXvY8KYBKvAmErEUYtmkA';
var options = {
  protocol: 'mqtts',
  // Assuming that the mqtt-ca certificate (https://www.thethingsnetwork.org/docs/applications/mqtt/quick-start.html) is in the same folder
  ca: [ fs.readFileSync('mqtt-ca.pem') ],
}

var client = new ttn.data.MQTT(region, appId, accessKey);

client.on('connect', function(connack) {
  console.log('[DEBUG]', 'Connect:', connack);
  console.log('[DEBUG]', 'Protocol:', client.mqtt.options.protocol);
  console.log('[DEBUG]', 'Host:', client.mqtt.options.host);
  console.log('[DEBUG]', 'Port:', client.mqtt.options.port);
});

client.on('error', function(err) {
  console.error('[ERROR]', err.message);
});

client.on('activation', function(deviceId, data) {
  console.log('[INFO] ', 'Activation:', deviceId, JSON.stringify(data, null, 2));
});

client.on('device', null, 'down/scheduled', function(deviceId, data) {
  console.log('[INFO] ', 'Scheduled:', deviceId, JSON.stringify(data, null, 2));
});

client.on('message', function(deviceId, data) {
console.info('[INFO] ', 'Message:', deviceId, JSON.stringify(data, null, 2));
  var connection = mysql.createConnection({
    host     : '127.0.0.1',
    user     : 'erwin50',
    password : 'mU6arak2brugastesaCeWUku',
    database : 'erwin50'
  });
    connection.connect();
  var messenge = "";
  var mustSend = false;
  var databaseId = 0;
  connection.query('SELECT messenge, id FROM berichten ORDER BY ID ASC LIMIT 1;', function (error, results, fields) {
      //if (error) throw error;
      if(typeof results[0] !== 'undefined' && results[0]) {
          console.log('Messenge is: ', results[0].messenge);
          messenge = results[0].messenge;
          mustSend = true;
          databaseId = results[0].id;
          var payload = {
            messenge: messenge
          };
          console.log('[DEBUG]', 'Sending:', JSON.stringify(payload));
          client.send(deviceId, payload);
          var sql = "DELETE berichten FROM berichten WHERE berichten.id = ?";
          
          var query = connection.query(sql, [databaseId], function(err, result) {
                console.log("Record Deleted!!");
                console.log(result);
                connection.end();
            });
          
          
      } else {
          connection.end();
      }
      
  });

  
   
 
   
  
  
      
      
    

});


