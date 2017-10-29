const { Gtk } = require('../');

describe('signals', () => {
  test('should be called when emitted', (done) => {
    // this test will timeout if the signal handler is never called
    const button = new Gtk.Button();
    button.connect('clicked', () => {
      done();
    });
    button.clicked();
  });

  test('signal connections should return a number that can be used to remove them', () => {
    const button = new Gtk.Button();
    const connectionId = button.connect('clicked', () => undefined);
    expect(typeof(connectionId)).toEqual('number');
  });

  test('a signal that passes parameters to it\'s callback should work', (done) => {
    const window = new Gtk.Window({ type: Gtk.WindowType.toplevel, title: "signal-test-window" });;
    const button = new Gtk.Button();
    button.connect('size-allocate', allocation => done());
    window.add(button);
    window.showAll();
  });

  test('connect() returns a number', () => {
    const button = new Gtk.Button();
    const result = button.connect('clicked', () => undefined);
    expect(typeof(result)).toEqual('number');
  });

  test('disconnect() will stop the connected callback from being called', (done) => {
    const button = new Gtk.Button();
    const connection = button.connect('clicked', () => {
      done('the signal callback shouldn\'t have been called!');
    });
    button.disconnect(connection);
    button.clicked();
    setTimeout(() => done(), 0);
  });
});
