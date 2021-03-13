import base64
import urllib.request
import pydle

import configparser

config = configparser.ConfigParser()
config.read('config.ini')
twitchConfig = config['twitch']

streamname = twitchConfig['Stream']
nickname   = twitchConfig['Nickname']
password   = twitchConfig['Password']


BaseIrcClass = pydle.featurize(pydle.features.RFC1459Support, pydle.features.IRCv3Support)

class MyOwnBot(BaseIrcClass):
  async def on_raw_004(self, msg):
    return
  async def on_connect(self):
    await self.join(streamname)
  async def on_message(self, target, nick, message):
    name = str(base64.urlsafe_b64encode(nick.encode("utf-8")), "utf-8")
    message = str(base64.urlsafe_b64encode(message.encode("utf-8")), "utf-8")
    target = str(base64.urlsafe_b64encode(target.encode("utf-8")), "utf-8")
    f = urllib.request.urlopen("http://localhost:11004/get?name="+name+"&text="+message+"&chan="+target)

client = MyOwnBot(nickname, realname=nickname)
client.run('irc.twitch.tv', 6667, password=password)
client.handle_forever()

