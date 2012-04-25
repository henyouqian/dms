var gCurrPage=0;
const gMatchPerPage=20;

$(document).ready(function(){
    $('#back').click(function(){
        back();
    });
    $('#add').click(function(){
        window.location.href='/dms/dev/match/add';
    });
    getMatches();
    $('button').button();
});

function back(){
    window.location.href='/dms/dev';
}

function getMatches(){
	$.getJSON('/dmsapi/dev/match/get', {offset:gCurrPage*gMatchPerPage, limit:gMatchPerPage}, function(json){
        err = json.error;
        if (err==DMSERR_NONE){
            table = $('#tbl');
            $('.datarow').remove();
            var str='';
            $(json.data).each(function(i){
                str+='<tr class=datarow>';
                str+='<td><button class=edit matchid='+this.id+' gameid='+this.gameid+' matchname='+this.name+' date='+this.date+'>Edit</button></td>';
                str+='<td class=matchname>'+this.name+'</td>';
                str+='<td class=gamename>'+this.gamename+'</td>';
                str+='<td class=date>'+this.date+'</td>';
                str+='</tr>';
            });
            table.append(str);
            $('.edit').click(function(){
                var obj = $(this);
                var matchid = obj.attr('matchid');
                var matchname = obj.attr('matchname');
                var gameid = obj.attr('gameid');
                var date = obj.attr('date');
                window.location.href = '/dms/dev/match/edit?matchid='+matchid+'&matchname='+matchname+'&gameid='+gameid+'&date='+date;
            });
            
            $('button').button();
        }else{
            errorProc(err);
        }
    });
}