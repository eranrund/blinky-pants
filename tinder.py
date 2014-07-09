import requests

def auth():
    r = requests.post('https://api.gotinder.com/auth', headers={
        'app_version': '633',
        'platform': 'android',
        'User-Agent': 'Tinder Android Version 2.2.3',
        'os_version': 19,
        'Content-Type': 'application/json; charset=utf-8',
        }, data='{"facebook_token": "CAAGm0PX4ZCpsBACxQKGumo1ubuZBJXFRz9tBjhKDy18EOHcsGlXK1e0gN1YNVdVpYj9iXAfdqEmYTer5UHKUZBrO6z3ZCdbs6ZAkMq8QnZCp6cCdlLdFDyz7BuI5McKK74vVZBSEuyWsbhFUrXK1UrqJZBv9BtIt9vHqI8Tacz7fcZAk64NWh3cLuCbHST6PF0S1ZAtZBUYdHmL0ZAg7noFRwnZAhrZC3TXDB54NWuc3R8RtSOkwZDZD"}')
    #print r.text
    return r.json()['token']

TOKEN = auth()


def ping():
    r = requests.post('https://api.gotinder.com/user/ping', headers={
        'app_version': '633',
        'platform': 'android',
        #'If-Modified-Since': 'Thu, 24 Apr 2014 23:24:41 GMT+00:00',
        'User-Agent': 'Tinder Android Version 2.2.3',
        'X-Auth-Token': TOKEN,
        'os_version': 19,
        'Content-Type': 'application/json; charset=utf-8',
    }, data='{"lon":-122.3955667,"lat":37.7775613}')
    print r.text

def recs(): 
    r = requests.post('https://api.gotinder.com/user/recs', headers={
        'app_version': '633',
        'platform': 'android',
        #'If-Modified-Since': 'Thu, 24 Apr 2014 23:24:41 GMT+00:00',
        'User-Agent': 'Tinder Android Version 2.2.3',
        'X-Auth-Token': TOKEN,
        'os_version': 19,
        'Content-Type': 'application/json; charset=utf-8',
    }, data='{"limit":40}')
    return r.json()

def like_or_pass(like_or_pass, id):
    r = requests.get('https://api.gotinder.com/{}/{}'.format(like_or_pass, id), headers={
        'app_version': '633',
        'platform': 'android',
        #'If-Modified-Since': 'Thu, 24 Apr 2014 23:24:41 GMT+00:00',
        'User-Agent': 'Tinder Android Version 2.2.3',
        'X-Auth-Token': TOKEN,
        'os_version': 19,
    })
    return r.json()

from pprint import pprint
import time, random
while True:
    r = recs()
    try:
        results = r['results']
    except:
        print r
        break

    print 'RESULTS:',len(results)

    for res in results:
        time.sleep(random.randint(10, 20) / 10)
        id = res['_id']    
        like = random.randint(0,15) != 3
        if like:
            print 'liking', id
            res2 = like_or_pass('like', id)
            if res2.get('match') != False:
                print 'MATCH!!'
                pprint(res)
                pprint(res2)
                print
                print
        else:
            print 'pass', id
            like_or_pass('pass', id)

    time.sleep(random.randint(1, 5))


