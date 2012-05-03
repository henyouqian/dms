$(document).ready(function(){
    $('#login').click(function() {
        login();
    });
    $('#register').click(function() {
        register();
    });
    $("#pw").keypress(function(event) {
        if ( event.which == 13 ) {
            login();
        }
    });
    $('button').button();
});

function login(){
	var email=$('#email').attr('value');
	var pw=$('#pw').attr('value');
	if ( email == '' ){
		alert('email == NULL');
	}else if ( pw == '' ){
		alert('password == NULL');
	}else{
		$.getJSON('/dmsapi/dev/login', {email:email, password:pw}, function(json){
            err = json.error;
			if (err==DMSERR_NONE){
				window.location.href='/dms/dev';
			}else if (err==DMSERR_EXIST){
				alert('email not exist');
			}else if (err==DMSERR_PASSWORD){
				alert('wrong password');
			}else{
                errorProc(err);
			}
		});
	}
}

function register(){
	window.location.href='/dms/dev/register';
}

