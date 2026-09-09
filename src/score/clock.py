import datetime as dt


class Clock:
    """A time source. Injected into collaborators so 'now' is controllable/testable."""

    def __init__(self, now=None):
        self._now = now or dt.datetime.now()

    def now(self):
        return self._now

    def is_weekend(self):
        return self._now.weekday() >= 5

    def is_after_hours(self):
        # Market closes 16:00. Weekend => always after hours.
        return self.is_weekend() or self._now.hour >= 16
