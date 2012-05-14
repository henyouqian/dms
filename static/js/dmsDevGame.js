$(document).ready(function(){
    $('#back').click(function(){
        back();
    });
    $('#add').click(function(){
        addGameDiag();
    });
    $('#appedit').click(function(){
        editAppDiag();
    });
    getGames();
    $('button').button();
});


function back(){
    window.location.href='/dms/dev';
}

function editAppDiag(){
    var html = '<table id=tbledit>\
                    <tr>\
                        <th>Name:</th>\
                        <td><input id=name type=text /></td>\
                    </tr>\
                </table>';

    var $dialog = $('<div class=ui-dialog></div>')
    .html(html)
    .dialog({
        title: 'Edit app name',
        modal: true,
        buttons: {
            "OK": function() {
                editApp();
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

//todo
function editApp(id){
    var name=$('#name').attr('value');
    var appid=_appid;
    if ( name == '' ){
        alert('name == NULL');
    }else if ( appid =='' ){
        alert('appid == NULL');
    }else{
        $.getJSON('/dmsapi/dev/game/edit', {id:id, name:name, order:order, appid:appid}, function(json){
            err = json.error;
            if (err==0){
                getGames();
            }else if (err==DMSERR_EXIST){
                alert('game name already exists');
            }else{
                errorProc(err);
            }
        });
    }
}

var diagHtml = '<table id=tbledit>\
                    <tr>\
                        <th>Name:</th>\
                        <td><input id=name type=text /></td>\
                    </tr>\
                    <tr>\
                        <th>Order DESC:</th>\
                        <td><input id=order type=checkbox /></td>\
                        <td></td>\
                    </tr>\
                </table>';

function addGameDiag(){
    var $dialog = $('<div class=ui-dialog></div>')
    .html(diagHtml)
    .dialog({
        title: 'Add game',
        modal: true,
        buttons: {
            "OK": function() {
                addGame();
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

function addGame(){
    var name=$('#name').attr('value');
    var order=$('#order').get(0).checked?1:0;
    var appid=_appid;
    if ( name == '' ){
        alert('name == NULL');
    }else if ( appid =='' ){
        alert('appid == NULL');
    }else{
        $.getJSON('/dmsapi/dev/game/add', {name:name, order:order, appid:appid}, function(json){
            err = json.error;
            if (err==0){
                getGames();
            }else if (err==DMSERR_RANGE){
                alert('game full');
            }else if (err==DMSERR_EXIST){
                alert('game name already exists');
            }else{
                errorProc(err);
            }
        });
    }
}

function editGameDiag(id, name, order){
    var $dialog = $('<div class=ui-dialog></div>')
    .html(diagHtml)
    .dialog({
        title: 'Edit game',
        modal: true,
        buttons: {
            "OK": function() {
                editGame(id);
                $( this ).dialog("close");
            },
            "Cancel": function() {
                $( this ).dialog("close");
            }
        },
        close: function(ev, ui) { $(this).dialog('destroy').remove(); },
        width: 360
    });
    $('#name').attr('value', name);
    $('#order')[0].checked = order==1?true:false;
}

function editGame(id){
    var name=$('#name').attr('value');
    var order=$('#order')[0].checked?1:0;
    var appid=_appid;
    if ( name == '' ){
        alert('name == NULL');
    }else if ( appid =='' ){
        alert('appid == NULL');
    }else{
        $.getJSON('/dmsapi/dev/game/edit', {id:id, name:name, order:order, appid:appid}, function(json){
            err = json.error;
            if (err==0){
                getGames();
            }else if (err==DMSERR_EXIST){
                alert('game name already exists');
            }else{
                errorProc(err);
            }
        });
    }
}

function delGameDiag(id, name){
    var $dialog = $('<div class=ui-dialog></div>')
    .html('Are you sure delete '+name+'?')
    .dialog({
        title: 'Delete game',
        modal: true,
        buttons: {
            "OK": function() {
                delGame(id);
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

function delGame(id){
    $.getJSON('/dmsapi/dev/game/delete', {id:id}, function(json){
        err = json.error;
        if (err==0){
            getGames();
        }else{
            errorProc(err);
        }
    });
}

function getGames(){
    $.getJSON('/dmsapi/dev/game/get', {appid:_appid}, function(json){
        err = json.error;
        if (err==DMSERR_NONE){
            table = $('#tbl');
            $('.datarow').remove();
            var str='';
            $(json.data).each(function(i){
                str+='<tr class=datarow>';
                str+='<td><button class=edit gameid='+this.id+' name='+this.name+' order='+this.order+' appid='+this.appid+'>Edit</button></td>';
                str+='<td class=name>'+this.name+'</td>';
                str+='<td>'+this.id+'</td>';
                var order='ASC';
                if (this.order==1){
                    order = 'DESC';
                }
                str+='<td class=order>'+order+'</td>';
                str+='<td>'+this.appid+'</td>';
                str+='<td><button class=del gameid='+this.id+' name='+this.name+'>Del</button></td>';
                str+='</tr>';
            });
            table.append(str);
            $('.edit').click(function(){
                var obj = $(this);
                var id = obj.attr('gameid');
                var name = obj.attr('name');
                var order = obj.attr('order');
                var appid = obj.attr('appid');
                editGameDiag(id, name, order, appid);
            });
            $('.del').click(function(){
                var obj = $(this);
                var id = obj.attr('gameid');
                var name = obj.attr('name');
                delGameDiag(id, name);
            });
        }else{
            errorProc(err);
        }
        $('button').button();
    });
}

