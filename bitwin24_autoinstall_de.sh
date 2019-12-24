# BitWin24 Masternode Setup Script V1.0 
#by mrx0rhk
#!/bin/bash
#
# Script will attempt to autodetect primary public IP address
# and generate masternode private key unless specified in command line
#
# Usage:
# bash bitwin24_autoinstall_de.sh
#

#Color codes
RED='\033[0;91m'
GREEN='\033[1;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

#TCP port
PORT=24072
RPC=24071

#Clear keyboard input buffer
function clear_stdin { while read -r -t 0; do read -r; done; }

#Delay script execution for N seconds
function delay { echo -e "${GREEN}Warte $1 Sekunde...${NC}"; sleep "$1"; }

#Stop daemon if it's already running
function stop_daemon {
    if pgrep -x 'bitwin24d' > /dev/null; then
        echo -e "${YELLOW}Versuche bitwin24d zu beenden${NC}"
        bitwin24-cli stop
        sleep 30
        if pgrep -x 'bitwin24d' > /dev/null; then
            echo -e "${RED}bitwin24d daemon läuft immer noch!${NC} \a"
            echo -e "${RED}Versuche zu beenden...${NC}"
            sudo pkill -9 bitwin24d
            sleep 30
            if pgrep -x 'bitwin24d' > /dev/null; then
                echo -e "${RED}Can't stop bitwin24d! Reboot and try again...${NC} \a"
                exit 2
            fi
        fi
    fi
}

#Process command line parameters
genkey=$1
clear

echo -e "${GREEN} ---------- BitWin24 MASTERNODE INSTALLER -----------
 |                                                  |
 |                                                  |
 |       Dieser Installer wird eine BitWin24        |
 |    Masternode installieren und konfigurieren     |
 |                                                  |
 |      Er wird außerdem aus Sicherheitsgründen     |
 |           fail2ban und ufw installieren.         |
 |                                                  |
 +--------------------------------------------------+
   ::::::::::::::::::::::::::::::::::::::::::::::::${NC}"
echo "Soll ein Masternode Private-Key generiert werden? [y/n]"
read DOSETUP

if [[ $DOSETUP =~ "n" ]] ; then
          read -e -p "Private Key eingeben:" genkey;
              read -e -p "Private Key bestätigen: " genkey2;
    fi

#Confirming match
  if [ $genkey = $genkey2 ]; then
     echo -e "${GREEN}Übereinstimmung! ${NC} \a" 
else 
     echo -e "${RED} Fehler: Private keys stimmen nicht überein. Erneut versuchen oder einen generieren lassen...${NC} \a";exit 1
fi
sleep .5
clear

# Determine primary public IP address
dpkg -s dnsutils 2>/dev/null >/dev/null || sudo apt-get -y install dnsutils
publicip=$(dig +short myip.opendns.com @resolver1.opendns.com)

if [ -n "$publicip" ]; then
    echo -e "${YELLOW}IP Addresse gefunden:" $publicip ${NC}
else
    echo -e "${RED}Fehler: Öffentliche IP Adresse nicht gefunden!${NC} \a"
    clear_stdin
    read -e -p "VPS IP Adresse eingeben: " publicip
    if [ -z "$publicip" ]; then
        echo -e "${RED}Fehler: Öffentliche IP muss angegeben werden. Erneut versuchen...${NC} \a"
        exit 1
    fi
fi
if [ -d "/var/lib/fail2ban/" ]; 
then
    echo -e "${GREEN}Erforderliche Pakete bereits installiert..${NC}"
else
   echo -e "${GREEN}Update System und installiere benötigte Pakete. Dies kann mehere Minuten dauern.${NC}"

sudo DEBIAN_FRONTEND=noninteractive apt-get update -y  
sudo apt-get -y upgrade 
sudo apt-get -y dist-upgrade 
sudo apt-get -y autoremove 
sudo apt-get -y install wget nano htop jq 
sudo apt-get -y install libzmq3-dev
sudo apt-get -y install libevent-dev -y 
sudo apt-get install unzip 
sudo apt install unzip 
sudo apt -y install software-properties-common 
sudo add-apt-repository ppa:bitcoin/bitcoin -y 
sudo apt-get -y update 2>/dev/null  >/dev/null
sudo apt-get -y install libdb4.8-dev libdb4.8++-dev -y 
sudo apt-get -y install libminiupnpc-dev 
sudo apt-get install -y unzip libzmq3-dev build-essential libssl-dev libboost-all-dev libqrencode-dev libminiupnpc-dev libdb4.8++ libdb4.8 libdb4.8-dev libdb4.8++-dev -y
   fi

#Network Settings
echo -e "${GREEN}Netzwerkeinstellungen werden konfiguriert...${NC}"
{
sudo apt-get install ufw -y
} &> /dev/null
echo -ne '[##                 ]  (10%)\r'
{
sudo apt-get update -y
} &> /dev/null
echo -ne '[######             ] (30%)\r'
{
sudo ufw default deny incoming
} &> /dev/null
echo -ne '[#########          ] (50%)\r'
{
sudo ufw default allow outgoing
sudo ufw allow ssh
} &> /dev/null
echo -ne '[###########        ] (60%)\r'
{
sudo ufw allow $PORT/tcp
sudo ufw allow $RPC/tcp
} &> /dev/null
echo -ne '[###############    ] (80%)\r'
{
sudo ufw allow 22/tcp
sudo ufw limit 22/tcp
} &> /dev/null
echo -ne '[#################  ] (90%)\r'
{
echo -e "${YELLOW}"
sudo ufw --force enable
echo -e "${NC}"
} &> /dev/null
echo -ne '[###################] (100%)\n'

#Generating Random Password for  JSON RPC
rpcuser=$(cat /dev/urandom | tr -dc 'a-zA-Z0-9' | fold -w 32 | head -n 1)
rpcpassword=$(cat /dev/urandom | tr -dc 'a-zA-Z0-9' | fold -w 32 | head -n 1)

#Create 2GB swap file
if grep -q "SwapTotal" /proc/meminfo; then
    echo -e "${GREEN}Überspringe Disk Swap Konfiguration...${NC} \n"
else
    echo -e "${YELLOW}Erstelle 2GB Disk Swap Datei. \nDies kann ein paar Minuten dauern!${NC} \a"
    touch /var/swap.img
    chmod 600 swap.img
    dd if=/dev/zero of=/var/swap.img bs=1024k count=2000
    mkswap /var/swap.img 2> /dev/null
    swapon /var/swap.img 2> /dev/null
    if [ $? -eq 0 ]; then
        echo '/var/swap.img none swap sw 0 0' >> /etc/fstab
        echo -e "${GREEN}Swap erfolgreich erstellt!${NC} \n"
    else
        echo -e "${RED}Berechtigungsfehler! Optionaler Swap wurde nicht erstellt.${NC} \a"
        rm /var/swap.img
    fi
fi
 
#Installing Daemon
echo -e "${GREEN}Downloade und installiere BitWin24 Deamon...${NC}"
cd ~
rm -rf /usr/local/bin/bitwin24*
wget https://github.com/BitWin24/bitwin24/releases/download/v0.0.6/bitwin24-1.0.0-x86_64-linux-gnu.tar.gz
tar -xzvf bitwin24-1.0.0-x86_64-linux-gnu.tar.gz
cd /root/bitwin24-1.0.0/bin/  2>/dev/null  >/dev/null
sudo chmod -R 755 bitwin24-cli  2>/dev/null  >/dev/null
sudo chmod -R 755 bitwin24d  2>/dev/null  >/dev/null
cp -p -r bitwin24d /usr/local/bin  2>/dev/null  >/dev/null
cp -p -r bitwin24-cli /usr/local/bin  2>/dev/null  >/dev/null
bitwin24-cli stop  2>/dev/null  >/dev/null
rm ~/bitwin24-1.0.0-x86_64-linux-gnu.tar.gz*  2>/dev/null  >/dev/null
 
sleep 5
 #Create datadir
 if [ ! -f ~/.bitwin24/bitwin24.conf ]; then 
 	sudo mkdir ~/.bitwin24  2>/dev/null  >/dev/null
 fi

cd ~
clear
echo -e "${YELLOW}Erstelle bitwin24.conf...${NC}"

# If genkey was not supplied in command line, we will generate private key on the fly
if [ -z $genkey ]; then
    cat <<EOF > ~/.bitwin24/bitwin24.conf
rpcuser=$rpcuser
rpcpassword=$rpcpassword
EOF

    sudo chmod 755 -R ~/.bitwin24/bitwin24.conf

    #Starting daemon first time just to generate a BitWin24 masternode private key
    bitwin24d -daemon 2>/dev/null  >/dev/null
sleep 7
while true;do
    echo -e "${YELLOW}Generiere Masternode Private Key...${NC}"
    genkey=$(bitwin24-cli masternode genkey)
    if [ "$genkey" ]; then
        break
    fi
	
sleep 7
done
    fi
    
    #Stopping daemon to create bitwin24.conf
    bitwin24-cli stop
    sleep 5
# Create cryptoverification.conf
cat <<EOF > ~/.bitwin24/bitwin24.conf
rpcuser=$rpcuser
rpcpassword=$rpcpassword
rpcallowip=127.0.0.1
rpcport=$RPC
port=$PORT
listen=1
server=1
daemon=1
logtimestamps=1
maxconnections=256
masternode=1
externalip=$publicip
bind=$publicip
masternodeaddr=$publicip
masternodeprivkey=$genkey
 
EOF
    bitwin24d -daemon
#Finally, starting daemon with new bitwin24.conf
printf '#!/bin/bash\nif [ ! -f "~/.bitwin24/bitwin24.pid" ]; then /usr/local/bin/bitwin24d -daemon ; fi' > /root/bitwin24auto.sh
chmod -R 755 bitwin24auto.sh
#Setting auto start cron job for bitwin24
if ! crontab -l | grep "bitwin24auto.sh"; then
    (crontab -l ; echo "*/5 * * * * /root/bitwin24auto.sh")| crontab -
fi

echo -e "========================================================================
${GREEN}Das BitWin24 Masternode Setup ist abgeschlossen!${NC}
========================================================================
Die Masternode wurde auf dieser IP Adresse installiert: ${GREEN}$publicip${NC}
Masternode Private Key: ${GREEN}$genkey${NC}
Sie können nun folgende Zeile zur masternode.conf Datei in ihrem Lokalen Wallet hinzufügen: 
======================================================================== \a"
echo -e "${GREEN}bitwin24_mn1 $publicip:$PORT $genkey TxId TxIdx${NC}"
echo -e "========================================================================
Markieren sie die ganze Zeile und kopieren sie sie in die Datei  
${GREEN}masternode.conf${NC} Datei und ersetzen sie:
    ${GREEN}bitwin24_mn1${NC} - Mit dem gewünschten Masternode Name (Alias)
    ${GREEN}TxId${NC} - Mit der Trankaktions ID vom Befehl -masternode outputs-
    ${GREEN}TxIdx${NC} - Mit dem Transaktions Index (0 or 1)
     Denken sie daran die masternode.conf zu speichern und das Wallet neu zu starten!
Um die Masternode am BitWin24 Netzwerk anzumelden, müssen sie diese vom lokalen Wallet starten. 
In diesem Schritt wird gleichzeitig geprüft ob die Collateral richtig hinterlegt wurde."

clear_stdin
read -p "*** Drücken sie eine beliebige Taste um fortzufahren ***" -n1 -s

echo -e "Warten sie bis das Wallet auf dieser VPS vollständig synchronisiert ist. Eventuell wird sich der 'Is Synced' Status zu
'true' ändern, das bedeutet der Sync ist abgeschlossen, dies kann aber
mehrere Minuten oder Stunden dauern - das ist Netzwerkabhängig.
Der initiale Masternode Status kann lauten:
    ${GREEN}Node just started, not yet activated${NC} or
    ${GREEN}Node is not in masternode list${NC}, das ist normal solange die Masternode nicht aktiviert ist.
"
clear_stdin
read -p "*** Drücken sie eine beliebige Taste um fortzufahren ***" -n1 -s

echo -e "
${GREEN}...hochscrollen um vorherige Screens zu sehen...${NC}
Hier sind ein paar nützliche Befehle für die Fehlersuche / Kontrolle:
========================================================================
Um die vom Installer erzeugt konfiguration anzusehen - bitwin24.conf:
${GREEN}cat ~/.bitwin24/bitwin24.conf${NC}
Das hier wurde generiert in der bitwin24.conf:
-------------------------------------------------${GREEN}"
echo -e "${GREEN}bitwin24_mn1 $publicip:$PORT $genkey TxId TxIdx${NC}"
cat ~/.bitwin24/bitwin24.conf
echo -e "${NC}-------------------------------------------------
NOTE: Um die bitwin24.conf zu bearbeiten, stoppen sie erst den bitwin24d daemon,
und bearbeiten sie dann die bitwin24.conf Datei und speichern sie sie in nano: (Ctrl-X + Y + Enter),
starten sie anschließende wieder den bitwin24d daemon :
zum Stoppen:              ${GREEN}bitwin24-cli stop${NC}
zum Starten:              ${GREEN}bitwin24d${NC}
zum Bearbeiten:           ${GREEN}nano ~/.bitwin24/bitwin24.conf ${NC}
um den Status zu checken: ${GREEN}bitwin24-cli masternode status${NC}
========================================================================
Ressourcenmonitor mit allen laufenden Prozessen:
                   ${GREEN}htop${NC}
========================================================================

${GREEN}Viel Spaß mit ihrer BitWin24 Masternode!${NC}

${RED}BitWin24 - Die erste echte Blockchain Lotterie${NC} 

"
rm ~/bitwin24_autoinstall_de.sh
