/*
 * NTP clock with WeMos D1 mini (ESP8266) and Nokia 5110 LCD
 */

// WiFi credentials
#define WLAN_SSID "foo"
#define WLAN_PASS "bar"

#define NTP_HOST "dk.pool.ntp.org"

// Nokia 5110 LCD display connections
#define RST D0
#define CE D5
#define DC D6
#define DIN D7
#define CLK D8

// https://github.com/baghayi/Nokia_5110
#include <Nokia_5110.h>
// https://github.com/SensorsIot
#include <NTPtimeESP.h>
// https://github.com/PaulStoffregen/Time
#include <TimeLib.h>
// https://github.com/JChristensen/Timezone
#include <Timezone.h>

Nokia_5110 lcd = Nokia_5110(RST, CE, DC, DIN, CLK);
NTPtime ntp(NTP_HOST);
strDateTime ntptime;
// Configure timezone
TimeChangeRule CEST={"CEST", Last, Sun, Mar, 2, 120};
TimeChangeRule CET={"CET", Last, Sun, Oct, 2, 60};
Timezone tz(CEST,CET);

void setup() {
	lcd.clear();
	lcd.setContrast(50);

	WiFi.mode(WIFI_STA);
	WiFi.begin(WLAN_SSID, WLAN_PASS);
	// Keep trying to connect
	while(WiFi.status() != WL_CONNECTED) {
		delay(500);
		lcd.clear();
		lcd.setCursor(0,0);
		lcd.println("Connecting...");
	}
	lcd.clear();
	lcd.setCursor(0,0);
	lcd.println("Connected!");
}

void loop() {
	// If we get disconnected, keep trying to reconnect
	while(WiFi.status() != WL_CONNECTED) {
		lcd.clear();
		lcd.setCursor(0,0);
		lcd.print("Reconnecting...");
	}

	// If TimeLib wants an update, send an NTP request
	if(timeStatus() != timeSet) {
		ntptime=ntp.getNTPtime(0,0);
		// If the NTP request was successful, use it as a basis for TimeLib
		if(ntptime.valid) {
			setTime(
				ntptime.hour,
				ntptime.minute,
				ntptime.second,
				ntptime.day,
				ntptime.month,
				ntptime.year);
		}
	}

	// The loop() function is probably going to run hundreds of times per
	// second. There's no need to update the LCD that often, and doing so will
	// make the LCD flicker which is annoying to watch. By comparing the
	// timestamp of our last update with the current value of now(), we can
	// check if a change has happened that should trigger an LCD update. In
	// practice this will happen once per second.
	static time_t last=0;
	if(timeStatus() == timeSet && last!=now()) {
		last=now();

		// We keep the system time as UTC and use a temporary variable to hold the local time
		time_t localTime=tz.toLocal(now());

		// Date formatting
		char chrDate[11];
		sprintf(chrDate,"%4d-%2d-%2d",
			year(localTime),
			month(localTime),
			day(localTime));

		// Time formatting
		char chrTime[9];
		sprintf(chrTime,"%02d:%02d:%02d",
			hour(localTime),
			minute(localTime),
			second(localTime));

		// Update the display with the date and time as formatted above
		lcd.clear();
		lcd.setCursor(0,0);
		lcd.println(chrDate);
		lcd.println(chrTime);
	}
}
