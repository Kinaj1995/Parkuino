# Install Server

Betriebsystem aktualisieren

```bash
sudo apt update && sudo apt upgrade -y
```

```bash
```

## Install MQTT

Mosquitto MQTT Server installieren

```bash
sudo apt install mosquitto mosquitto-clients -y
```

MQTT Server als Dienst aktivieren und die Version prüfen
```bash
sudo systemctl enable mosquitto.service
mosquitto -v
```

MQTT Benutzer erstellen mit dem Namen "admin"
```bash
sudo mosquitto_passwd -c /etc/mosquitto/passwd admin
```

Für dieses Projekt haben wir uns für folgende Logindaten entschieden:
Benutzername: admin
Passwort: 6010Kriens

Nun noch eine Konfigurationsfile erstellt werden damit der Server von ausserhalb erreichbar ist.
```bash
sudo nano /etc/mosquitto/conf.d/connect.conf
```

```text
per_listener_settings true
allow_anonymous false
listener 1883 
password_file /etc/mosquitto/passwd
```


## Install NodeRED


```bash
bash <(curl -sL https://raw.githubusercontent.com/node-red/linux-installers/master/deb/update-nodejs-and-nodered)
```

Um ein Dashboard verwenden zu können muss noch dei Pallete "node-red-dashboard" installiert werden.

NodeRED als Dienst aktivieren
```bash
sudo systemctl enable nodered.service
sudo systemctl start nodered.service
```
