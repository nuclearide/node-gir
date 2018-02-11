const { Gtk } = require('../');

/*
 * These tests check that the gtk main loop integration doesn't block
 * setTimeout, nextTick and Promises (i.e. V8 macro tasks and micro tasks).
 *
 * These tests will (unfortunately) not terminate if the main loop integration
 * does block these. It seems like Jest is unable to kill the test via a timeout
 * due to the test blocking (when the loop integration doesn't work!).
 *
 * So if your tests aren't halting, and this file contains the tests that never halt,
 * then the reason is likely the event loop integration in `src/loop.cpp`.
 */
describe('Gtk main loop integration', () => {
  test('it should not block setTimeout from popping', (done) => {
    setTimeout(() => {
      Gtk.mainQuit();
      done();
    }, 0);
    Gtk.main();
  });

  test('it should not block nextTick from popping', (done) => {
    process.nextTick(() => {
      Gtk.mainQuit();
      done();
    }, 0);
    Gtk.main();
  });

  test('it should not block Promises from resolving', (done) => {
    const promise = new Promise(resolve => resolve('dummy-value'));
    promise.then((result) => {
      Gtk.mainQuit();
      expect(result).toEqual('dummy-value');
      done();
    });
    Gtk.main();
  });
});
