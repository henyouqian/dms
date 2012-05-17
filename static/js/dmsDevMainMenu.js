$(document).ready(function(){
    $('#logout').click(function() {
        logout();
    });
    $('#secretkey').click(function() {
        secretkey();
    });
    $('#add').click(function(){
        addAppDiag();
    });
    getApps();
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

function getApps(){
    $.getJSON('/dmsapi/dev/app/get', function(json){
        err = json.error;
        if (err==DMSERR_NONE){
            table = $('#tbl');
            $('.datarow').remove();
            var str='';
            $(json.data).each(function(i){
                str+='<tr class=datarow>';
                str+='<td><button class=edit appid='+this.id+' name='+this.name+'>Edit</button></td>';
                str+='<td class=name>'+this.name+'</td>';
                str+='<td><button class=del appid='+this.id+' name='+this.name+'>Del</button></td>';
                str+='</tr>';
            });
            table.append(str);
            $('.edit').click(function(){
                var obj = $(this);
                var appid = obj.attr('appid');
                var name = obj.attr('name');
                editAppView(appid, name);
            });
            $('.del').click(function(){
                var obj = $(this);
                var id = obj.attr('appid');
                var name = obj.attr('name');
                delAppDiag(id, name);
            });
        }else{
            errorProc(err);
        }
        $('button').button();
    });
}

var diagHtml = '<table id=tbledit>\
                    <tr>\
                        <th>Name:</th>\
                        <td><input id=name type=text /></td>\
                    </tr>\
                </table>';

function addAppDiag(){
    var $dialog = $('<div class=ui-dialog></div>')
    .html(diagHtml)
    .dialog({
        title: 'Add app',
        modal: true,
        buttons: {
            "OK": function() {
                addApp();
                $( this ).dialog("close");
            },
            "Cancel": function() {
                $( this ).dialog("close");
            }
        },
        close: function(ev, ui) { $(this).dialog('destroy').remove(); },
        width: 360
    });
}

function addApp(){
    var name=$('#name').attr('value');
    if ( name == '' ){
        alert('name == NULL');
    }else{
        $.getJSON('/dmsapi/dev/app/add', {name:name}, function(json){
            err = json.error;
            if (err==0){
                getApps();
            }else if (err==DMSERR_RANGE){
                alert('app full');
            }else if (err==DMSERR_EXIST){
                alert('app name already exists');
            }else{
                errorProc(err);
            }
        });
    }
}

function editAppView(appid, appname){
    window.location.href='/dms/dev/game?appid='+appid+'&appname='+appname;
}

function delAppDiag(id, name){
    var $dialog = $('<div class=ui-dialog></div>')
    .html('Are you sure to delete '+name+'?')
    .dialog({
        title: 'Delete app',
        modal: true,
        buttons: {
            "OK": function() {
                delApp(id);
                $( this ).dialog("close");
            },
            "Cancel": function() {
                $( this ).dialog("close");
            }
        },
        close: function(ev, ui) { $(this).dialog('destroy').remove(); },
        width: 360
    });
}

function delApp(id){
    $.getJSON('/dmsapi/dev/app/delete', {id:id}, function(json){
        err = json.error;
        if (err==0){
            getApps();
        }else{
            errorProc(err);
        }
    });
}

