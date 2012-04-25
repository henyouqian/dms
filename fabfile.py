import sys
import time
from fabric.api import env, sudo, put, local, get

# this should be a path to your SSH key for the EC2 instance
keyPath = '/Users/liwei/keycoco.pem'
serverPath1= '/root/www'
serverPath2='/root/www'
packName='vote'
serverIp='173.230.148.198'

def livecoco():
	env.hosts = ['176.32.86.204']
	env.user = 'ec2-user'
	env.key_filename = '/Users/liwei/keycoco.pem'
    
def moregames():
	pack='moregames'
	path='/home/ec2-user/www'
	local('tar -cvf '+pack+'.tar.gz '+pack)
	put(pack+'.tar.gz', path)
	sudo('tar -xvf '+path+'/'+pack+'.tar.gz -C '+path)
	sudo('rm -f '+path+'/'+pack+'.tar.gz')

def live():
    # DNS entry of our instance
    env.hosts = [serverIp]
    env.user = 'root'
    #env.key_filename = keyPath
    env.password = 'Nmmgb808313'

def fuckserver():
    local('tar -cvf '+packName+'.tar.gz '+packName)
    put(packName+'.tar.gz', serverPath1)
    sudo('tar -xvf '+serverPath1+'/'+packName+'.tar.gz -C '+serverPath2)
    sudo('rm -f '+serverPath1+'/'+packName+'.tar.gz')
	
def dms():
	pack='dms'
	local('tar -cvf '+pack+'.tar.gz '+pack)
	put(pack+'.tar.gz', serverPath1)
	sudo('tar -xvf '+serverPath1+'/'+pack+'.tar.gz -C '+serverPath2)
	sudo('rm -f '+serverPath1+'/'+pack+'.tar.gz')

def nginxconf():
    put('/usr/local/nginx/conf/nginx.conf', serverPath1)
    sudo('mv -f '+serverPath1+'/nginx.conf /usr/local/nginx/conf')
    
def realart():
	pack='realart'
	local('tar -cf '+pack+'.tar.gz '+pack)
	put(pack+'.tar.gz', serverPath1)
	sudo('tar -xf '+serverPath1+'/'+pack+'.tar.gz -C '+serverPath2)
	sudo('rm -f '+serverPath1+'/'+pack+'.tar.gz')
	
def luckydraw():
	pack='luckydraw'
	local('tar -cf '+pack+'.tar.gz '+pack)
	put(pack+'.tar.gz', serverPath1)
	sudo('tar -xf '+serverPath1+'/'+pack+'.tar.gz -C '+serverPath2)
	sudo('rm -f '+serverPath1+'/'+pack+'.tar.gz')
    
def lucky2():
	pack='lucky2'
	local('tar -cf '+pack+'.tar.gz '+pack)
	put(pack+'.tar.gz', serverPath1)
	sudo('tar -xf '+serverPath1+'/'+pack+'.tar.gz -C '+serverPath2)
	sudo('rm -f '+serverPath1+'/'+pack+'.tar.gz')

	
def getvote():
	get('/root/www/vote/backup/backup_20111202_11am.sql', './')