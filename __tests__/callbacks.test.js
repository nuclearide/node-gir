const { load } = require('../');

const GLib = load('GLib');
const Gio = load('Gio');

describe('callbacks', () => {
  test('callbacks are called with arguments', (done) => {
    const loop = new GLib.MainLoop(null, false);
    const task = Gio.Task.new(undefined, undefined, (_, result) => {
      loop.quit();
      expect(result).not.toBe(undefined);
      expect(result).not.toBe(null);
      done();
    });
    task.returnBoolean(true);
    loop.run();
  });
});
