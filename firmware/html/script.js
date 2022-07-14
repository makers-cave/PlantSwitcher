
var currentConfig = {};
function ParseParameter(name)
{
    return (location.search.split(name + '=')[1] || '').split('&')[0];
}
function setUI(){
    if (currentConfig){
        if (currentConfig.MODE == '1'){
            if ($('#auto').hasClass('status-red')){
                $('#auto').removeClass('status-red');
                $('#auto').addClass('status-green');
            }
            if ($('#manual').hasClass('status-green')){
                $('#manual').removeClass('status-green');
                $('#manual').addClass('status-red');
            }
        } else {
            if ($('#manual').hasClass('status-red')){
                $('#manual').removeClass('status-red');
                $('#manual').addClass('status-green');
            }
            if ($('#auto').hasClass('status-green')){
                $('#auto').removeClass('status-green');
                $('#auto').addClass('status-red');
            }
        }
        if (currentConfig.LINE == '1'){
            if (!$('#line1').hasClass('status-green')){
                $('#line1').removeClass('status-red');
                $('#line1').addClass('status-green');
                $('#line2').removeClass('status-green');
                $('#line2').addClass('status-red');
            }
        } else {
            if (!$('#line2').hasClass('status-green')){
                $('#line2').removeClass('status-red');
                $('#line2').addClass('status-green');
                $('#line1').removeClass('status-green');
                $('#line1').addClass('status-red');
            }
        }
        $('#ct').val(currentConfig.CLEARTIME);
        $('#l1a').val(currentConfig.L1ANGLE);
        $('#l2a').val(currentConfig.L2ANGLE);
        $('#l1r').val(currentConfig.L1RUNTIME);
        $('#l2r').val(currentConfig.L2RUNTIME);
    }
}
function wsConnect() {
    if ('WebSocket' in window) {

// accept ?target=10.0.0.123 to make a WS connection to another device
        if (target = ParseParameter('target')) {
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
                currentConfig = JSON.parse(event.data);
                console.log(currentConfig);
                setUI()
            }
        };

        ws.onclose = function() {
            console.log("Error Opening WS");
        };

    } else {
        alert('WebSockets is NOT supported by your Browser! You will need to upgrade your browser.');
    }
}
function sendMessage(message){
    ws.send(message);
}
function feed() {
    if ($('#home').is(':visible')) {
        sendMessage(JSON.stringify({'CMD':'G'}));
        setTimeout(function() {
            feed();
        }, 1000);
    }
}

function onAutoClick() {
    $('#line1').prop('disabled', true);
    $('#line2').prop('disabled', true);
    currentConfig.MODE = 1;
    sendMessage(JSON.stringify({'CMD':'M','MODE':1}));
}
function OnManualClick() {
    $('#line1').prop('disabled', false);
    $('#line2').prop('disabled', false);
    currentConfig.MODE = 0;
    sendMessage(JSON.stringify({'CMD':'M','MODE':0}));
}
function onConfigureClick() {
    $('#home').addClass('hidden');
    $('#configure').removeClass('hidden');
}
function onLine1Click() {
    sendMessage(JSON.stringify({'CMD':'L','LINE':1}));
}
function onLine2Click() {
    sendMessage(JSON.stringify({'CMD':'L','LINE':2}));
}
function onSaveClick() {
    currentConfig.CLEARTIME = $('#ct').val();
    currentConfig.L1ANGLE = $('#l1a').val();
    currentConfig.L2ANGLE = $('#l2a').val();
    currentConfig.L1RUNTIME = $('#l1r').val();
    currentConfig.L2RUNTIME = $('#l2r').val();
    sendMessage(JSON.stringify({'CMD':'S','CFG':currentConfig}));
}
function onBackClick() {
    $('#home').removeClass('hidden');
    $('#configure').addClass('hidden');
    feed();
}
function ctClick(direction){
    if (direction == 0){
        $('#ct').val( parseFloat($('#ct').val()) - 100);
    } else {
        $('#ct').val( parseFloat($('#ct').val())  + 100);
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
        $('#l1r').val( parseFloat($('#l1r').val()) - 100);
    } else {
        $('#l1r').val( parseFloat($('#l1r').val())  + 100);
    }
}
function l2rClick(direction){
    if (direction == 0){
        $('#l2r').val( parseFloat($('#l2r').val()) - 100);
    } else {
        $('#l2r').val( parseFloat($('#l2r').val())  + 100);
    }
}