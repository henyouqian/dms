$(document).ready(function(){
    $('#back').click(function(){
        back();
    });
    $('#edit').click(function(){
        edit();
    });
    $('#delete').click(function(){
        del();
    });
    //test minDate: +1 -> +0
    $('#date').datepicker({dateFormat:'yy-mm-dd', minDate: getCurrUTCDate()});
    $('button').button();
});

function back(){
    window.location.href='/dms/dev/match';
}

function getCurrUTCDate(){
	a = new Date(Date());
	b = new Date(a.getUTCFullYear(), a.getUTCMonth(), a.getUTCDate(), a.getUTCHours(), a.getUTCMinutes())
	return b;
}

function edit(){
    var matchid=gMatchid
	var name=$('#name').attr('value');
    var gameid=$('#gameselect').attr('value');
    var date=$('#date').attr('value');
    if ( name == '' ){
        alert('name == NULL');
    }else if ( gameid == '' ){
        alert('gameid == NULL');
    }else if ( date == '' ){
        alert('date == NULL');
    }else{
        $.getJSON('/dmsapi/dev/match/edit', {matchid:matchid, name:name, gameid:gameid, date:date}, function(json){
            err = json.error;
            if (err==DMSERR_NONE){
                window.location.href='/dms/dev/match';
            }else{
                errorProc(err);
            }
        });
    }
}

function del(){
    var matchid=gMatchid
    $.getJSON('/dmsapi/dev/match/delete', {matchid:matchid}, function(json){
        err = json.error;
        if (err==DMSERR_NONE){
            window.location.href='/dms/dev/match';
        }else{
            errorProc(err);
        }
    });
}