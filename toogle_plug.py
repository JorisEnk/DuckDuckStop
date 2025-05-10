import appdaemon.plugins.hass.hassapi as hass

class TogglePlug(hass.Hass):

    def initialize(self):
        self.log("TogglePlug App Initialized")
        # Get the entity ID from config
        self.entity = self.args.get("entity_id")
        if not self.entity:
            self.log("Error: No entity_id defined!", level="ERROR")
            return

        # Start the toggle loop
        self.toggle_state = True
        self.run_every(self.toggle_plug, self.datetime(), 5)

    def toggle_plug(self, kwargs):
        if self.toggle_state:
            self.turn_on(self.entity)
            self.log(f"Turned ON {self.entity}")
        else:
            self.turn_off(self.entity)
            self.log(f"Turned OFF {self.entity}")
        self.toggle_state = not self.toggle_state
