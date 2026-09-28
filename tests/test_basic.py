import unittest
from client import connect, command, unique_key


class TestBasic(unittest.TestCase):
    def setUp(self):
        self.sock = connect()

    def tearDown(self):
        self.sock.close()

    def test_set_then_get(self):
        key = unique_key("basic")
        self.assertEqual(command(self.sock, f"SET {key} hello"), "SET operation OK")
        self.assertEqual(command(self.sock, f"GET {key}"), "hello")

    def test_get_missing_key(self):
        key = unique_key("missing")
        self.assertEqual(command(self.sock, f"GET {key}"), "Not found")

    def test_delete_then_get(self):
        key = unique_key("delete")
        command(self.sock, f"SET {key} temp")
        self.assertEqual(command(self.sock, f"DEL {key}"), "DEL operation OK")
        self.assertEqual(command(self.sock, f"GET {key}"), "Not found")

    def test_set_twice_updates_value(self):
        # Guards against the duplicate-key bug
        key = unique_key("update")
        command(self.sock, f"SET {key} first")
        command(self.sock, f"SET {key} second")
        self.assertEqual(command(self.sock, f"GET {key}"), "second")
        command(self.sock, f"DEL {key}")
        self.assertEqual(command(self.sock, f"GET {key}"), "Not found")

    def test_reinsert_after_delete(self):
        # The new entry should land in (or probe past) the tombstone correctly
        key = unique_key("reinsert")
        command(self.sock, f"SET {key} old")
        command(self.sock, f"DEL {key}")
        command(self.sock, f"SET {key} new")
        self.assertEqual(command(self.sock, f"GET {key}"), "new")

    def test_many_keys_survive_resize(self):
        # Enough keys to force several resizes from the initial capacity of 16
        prefix = unique_key("resize")
        for i in range(200):
            command(self.sock, f"SET {prefix}_{i} v{i}")
        for i in range(200):
            self.assertEqual(command(self.sock, f"GET {prefix}_{i}"), f"v{i}")


if __name__ == "__main__":
    unittest.main()