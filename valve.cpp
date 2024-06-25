void valve() {
    if (all_Timer - time_previous >= interval) {
        digitalWrite(relay, LOW);  
        delay(1000); 
        digitalWrite(relay, HIGH); 
        time_previous = all_Timer; 
    }
}