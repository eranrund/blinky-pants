import requests, lxml.html
import smtplib
from email.mime.text import MIMEText


try:
    # autoreturn
    r = requests.get(r'http://autoreturn.com/app/results?m=fyv&region=sf-ca&license=2fhl782&state=CA&v=&vehicle_make=&vehicle_model=&vehicle_year=&vehicle_color=&tm=&td=&ty=&search.x=39&search.y=10&search=search')
    assert r.status_code == 200
    page = lxml.html.fromstring(r.text)

    txt = page.cssselect('div#primary > p')[0].text_content()
    assert '3 vehicles were found matching' in txt, txt

    trs = page.cssselect('table.searchresults tr.heading')
    assert len(trs) == 3, len(trs)

    assert trs[0].cssselect('td')[-1].text_content().encode('ascii', 'ignore') == '6/20/144:53 PM',trs[0].cssselect('td')[-1].text_content().encode('ascii', 'ignore')
    assert trs[1].cssselect('td')[-1].text_content().encode('ascii', 'ignore') == '2/4/149:54 AM',trs[1].cssselect('td')[-1].text_content().encode('ascii', 'ignore')
    assert trs[2].cssselect('td')[-1].text_content().encode('ascii', 'ignore') == '10/19/1311:48 PM', trs[2].cssselect('td')[-1].text_content().encode('ascii', 'ignore')


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
    assert el[0].text_content().encode('ascii', 'ignore').strip() == 'The Plate entered has a balance of $0.00'
except:
    import traceback; traceback.print_exc()
    msg1 = MIMEText('CHECK FAILED !!!!')
    msg1['Subject'] = 'RV CHECKER - FAILED !!!!'
    msg1['From'] = 'rv.omercy@gmail.com'
    msg1['To'] = 'eranrund@gmail.com'

    msg2 = MIMEText('CHECK FAILED !!!!')
    msg2['Subject'] = 'RV CHECKER - FAILED !!!!'
    msg2['From'] = 'rv.omercy@gmail.com'
    msg2['To'] = 'shlomo.zippel@gmail.com'

else:
    msg1 = MIMEText('CHECK OK')
    msg1['Subject'] = 'RV CHECKER - OKAY'
    msg1['From'] = 'rv.omercy@gmail.com'
    msg1['To'] = 'eranrund@gmail.com'

    msg2 = MIMEText('CHECK OK')
    msg2['Subject'] = 'RV CHECKER - OKAY'
    msg2['From'] = 'rv.omercy@gmail.com'
    msg2['To'] = 'shlomo.zippel@gmail.com'

s = smtplib.SMTP('localhost')
s.sendmail(msg1['From'], msg1['To'], msg1.as_string())
s.sendmail(msg2['From'], msg2['To'], msg2.as_string())
s.quit()
