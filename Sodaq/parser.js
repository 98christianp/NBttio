function main(params, callback){
  
    /*
    The geoloc variable returns the latest location the device has been seen at. If there is no GPS fix available,
    the datagram will be 14 bytes and if there is GPS location it will be 26. We deal with these below.
    */

    // "D9s9RWi9S-zwoS2GqNQq5mZFvYTv6BEiQw83ik5HMR4" is the token, replace with your own
    var geoloc = thethingsAPI.thingRead('D9s9RWi9S-zwoS2GqNQq5mZFvYTv6BEiQw83ik5HMR4', '$geo', {limit: 1}, (err, result) => {
        if (err) return callback(err);
      
        var geoloc = result[0];
    
    if(params.data.length == 14){
      var result = [
        {
            "key": "message",
            "value": params.data
        },      
        {
            "key": "signal",
            //"value": params.data
            "value": ((parseInt(params.data.substring(0,2), 16))/31 *100).toFixed(1),
            "geo":
            {
                "lat":geoloc.value.coordinates[0],
                "long":geoloc.value.coordinates[1]
             }
        },
        {
             "key":"Cell_ID",
             "value": parseInt(params.data.substring(2,10),16),
                  "geo":
            {
                "lat":geoloc.value.coordinates[0],
                "long":geoloc.value.coordinates[1]
             }
        },
        {
             "key":"Signal_Power",
             "value":- parseInt(params.data.substring(10,14),16),
                "geo":
            {
                "lat":geoloc.value.coordinates[0],
                "long":geoloc.value.coordinates[1]
             }
        }
      ]
    }
    else {
      var result = [
        {
            "key": "message",
            "value": params.data,
        },      
        
        {
            "key": "signal",
            //"value": params.data
            "value": ((parseInt(params.data.substring(0,2), 16))/31 *100).toFixed(1),
           "geo":
            {
                "lat":parseInt(params.data.substring(2,8),16)/10000,
                "long":parseInt(params.data.substring(8,14),16)/10000
             }
        },
        {
             "key":"Cell_ID",
             "value": parseInt(params.data.substring(14,22),16),
             "geo":
            {
                "lat":parseInt(params.data.substring(2,8),16)/10000,
                "long":parseInt(params.data.substring(8,14),16)/10000
             }
        },
        {
             "key":"Signal_Power",
             "value": - parseInt(params.data.substring(22,26),16)/10,
          "geo":
            {
                "lat":parseInt(params.data.substring(2,8),16)/10000,
                "long":parseInt(params.data.substring(8,14),16)/10000
             }
        }
      ]
    }
        callback(null, result); 
  
      });
    
  
    
  },