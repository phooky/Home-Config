#!/usr/bin/python

from github3 import authorize, login
from getpass import getuser, getpass
from os.path import exists, expanduser
from subprocess import Popen

CREDENTIALS_PATH = expanduser('~/.clone_repo.token')

def cloneAllRepos(gh):
    for repo in gh.iter_repos():
        print("Cloning {0}...".format(repo.name))
        clone_url = repo.ssh_url
        cmd = ['git', 'clone', clone_url]
        p = Popen(cmd)
        p.wait()
        print("Cloned.")

def createCredentials():
    note = 'Repo cloner'
    note_url = 'http://phooky.name'
    scopes = [ 'user', 'repo' ]
    user = getuser()
    password = getpass('GitHub password for {0}: '.format(user))
    auth = authorize(user, password, scopes, note, note_url)
    with open(CREDENTIALS_PATH, 'w') as f:
        f.write(str(auth.token) + '\n' + str(auth.id) + '\n')
        f.close()

if not exists(CREDENTIALS_PATH):
    createCredentials()

with open(CREDENTIALS_PATH, 'r') as f:
    token = f.readline().strip()
    id = f.readline().strip()
    gh = login(token = token)
    cloneAllRepos(gh)
