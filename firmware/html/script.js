

function wsConnect() {
    if ('WebSocket' in window) {

// accept ?target=10.0.0.123 to make a WS connection to another device
        if (target = param('target')) {
// 
        } else {
            target = document.location.host;
        }

        // Open a new web socket and set the binary type
        ws = new WebSocket('ws://' + target + '/ws');
        ws.binaryType = 'arraybuffer';

        ws.onopen = function() {
            feed();
        };

        ws.onmessage = function (event) {
            if(typeof event.data === "string") {
                console.log( event.data);
            }
        };

        ws.onclose = function() {
            console.log("Error Opening WS");
        };

    } else {
        alert('WebSockets is NOT supported by your Browser! You will need to upgrade your browser.');
    }
}
function feed() {
    if ($('#home').is(':visible')) {
        message = 'getconfig';
        ws.send(message);

        setTimeout(function() {
            feed();
        }, 1000);
    }
}

function onAutoClick() {
    $('#line1').prop('disabled', true)
    $('#line2').prop('disabled', true)
}
function OnManualClick() {
    $('#line1').prop('disabled', false)
    $('#line2').prop('disabled', false)
}
function onConfigureClick() {
    $('#home').addClass('hidden');
    $('#configure').removeClass('hidden');
}
function onLine1Click() {
    
}
function onLine2Click() {
    
}
function onSaveClick() {
    
}
function onBackClick() {
    $('#home').removeClass('hidden');
    $('#configure').addClass('hidden');
}
function ctClick(direction){
    if (direction == 0){
        $('#ct').val( parseFloat($('#ct').val()) - 0.1);
    } else {
        $('#ct').val( parseFloat($('#ct').val())  + 0.1);
    }
}
function l1aClick(direction){
    if (direction == 0){
        $('#l1a').val( parseFloat($('#l1a').val()) - 1);
    } else {
        $('#l1a').val( parseFloat($('#l1a').val())  + 1);
    }
}
function l2aClick(direction){
    if (direction == 0){
        $('#l2a').val( parseFloat($('#l2a').val()) - 1);
    } else {
        $('#l2a').val( parseFloat($('#l2a').val())  + 1);
    }
}
function l1rClick(direction){
    if (direction == 0){
        $('#l1r').val( parseFloat($('#l1r').val()) - 0.1);
    } else {
        $('#l1r').val( parseFloat($('#l1r').val())  + 0.1);
    }
}
function l2rClick(direction){
    if (direction == 0){
        $('#l2r').val( parseFloat($('#l2r').val()) - 0.1);
    } else {
        $('#l2r').val( parseFloat($('#l2r').val())  + 0.1);
    }
}