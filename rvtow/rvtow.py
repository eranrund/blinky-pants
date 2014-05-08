import requests, lxml.html
import smtplib
from email.mime.text import MIMEText


try:
    # autoreturn
    r = requests.get(r'http://autoreturn.com/app/results?m=fyv&region=sf-ca&license=2fhl782&state=CA&v=&vehicle_make=&vehicle_model=&vehicle_year=&vehicle_color=&tm=&td=&ty=&search.x=39&search.y=10&search=search')
    assert r.status_code == 200
    page = lxml.html.fromstring(r.text)

    txt = page.cssselect('div#primary > p')[0].text_content()
    assert '2 vehicles were found matching' in txt

    trs = page.cssselect('table.searchresults tr.heading')
    assert len(trs) == 2

    assert trs[0].cssselect('td')[-1].text_content().encode('ascii', 'ignore') == '2/4/149:54 AM'
    assert trs[1].cssselect('td')[-1].text_content().encode('ascii', 'ignore') == '10/19/1311:48 PM'


    r = requests.post('http://wmq.etimspayments.com/pbw/inputAction.doh', data={
        'clientcode':'19',
        'requestType':'submit',
        'requestCount':'1',
        'clientAccount':'5',
        'ticketNumber':'',
        'plateNumber':'2fhl782',
        'statePlate':'CA',
        'submit':'  Search for citations  ',
    })
    assert r.status_code == 200
    page = lxml.html.fromstring(r.text)

    el = page.cssselect('li.error')
    assert len(el) == 1
    assert el[0].text_content().encode('ascii', 'ignore').strip() == 'The Plate entered has a balance of $0.00x'
except:
    import traceback; traceback.print_exc()
    msg = MIMEText('CHECK FAILED !!!!')
    msg['Subject'] = 'RV CHECKER - FAILED !!!!'
    msg['From'] = 'rv.omercy@gmail.com'
    msg['To'] = 'eranrund@gmail.com, shlomo.zippel@gmail.com'
else:
    msg = MIMEText('CHECK OK')
    msg['Subject'] = 'RV CHECKER - OKAY'
    msg['From'] = 'rv.omercy@gmail.com'
    msg['To'] = 'eranrund@gmail.com, shlomo.zippel@gmail.com'


s = smtplib.SMTP('localhost')
s.sendmail(msg['From'], msg['To'], msg.as_string())
s.quit()
