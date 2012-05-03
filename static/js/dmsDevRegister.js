$(document).ready(function(){
    $('#register').click(function() {
        register();
    });
    $('#back').click(function() {
        back();
    });
    $("#pw2").keypress(function(event) {
        if ( event.which == 13 ) {
            register();
        }
    });
    $('button').button();
});

function register(){
	var email=$('#email').attr('value');
	var pw1=$('#pw1').attr('value');
	var pw2=$('#pw2').attr('value');
	if ( email == '' ){
		alert('email == NULL');
	}else if ( pw1 != pw2 ){
		alert('pw1 != pw2');
	}else if ( pw1 == '' ){
		alert('pw1 == NULL');
	}else{
		$.getJSON('/dmsapi/dev/register', {email:email, password:pw1}, function(json){
            err = json.error;
			if (err==DMSERR_NONE){
				window.location.href='/dms/dev';
			}else if (err==DMSERR_EXIST){
				alert('email already exists');
			}else{
				errorProc(err);
			}
		});
	}
}

function back(){
	window.location.href='/dms/dev';
}