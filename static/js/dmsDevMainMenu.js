$(document).ready(function(){
    $('#logout').click(function() {
        logout();
    });
    $('#secretkey').click(function() {
        secretkey();
    });
    $('#game').click(function() {
        game();
    });
    $('#match').click(function() {
        match();
    });
	$('button').button();
});

function logout(){
	$.getJSON('/dmsapi/dev/logout', function(json){
		if (json.error==DMSERR_NONE){
			window.location.href='/dms/dev';
		}
	});
}

function secretkey(){
	$.getJSON('/dmsapi/dev/getsecretkey', function(json){
        err = json.error;
		if (err==DMSERR_NONE){
			alert(json.secretkey);
		}else{
			errorProc(err);
		}
	});
}

function game(){
	window.location.href='/dms/dev/game';
}

function match(){
	window.location.href='/dms/dev/match';
}
