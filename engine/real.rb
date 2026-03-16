# frozen_string_literal: true

# # Real — A Ruby Game Framework
#
# Real is a lightweight Ruby game framework built on top of
# [raylib](https://www.raylib.com/), exposed through mruby bindings.
#
# ## Quick Start
#
# ```ruby
# Real::Window.size  = Real::Vector2.new(800.0, 600.0)
# Real::Window.title = "My Game"
#
# Real::Window.open do
#   # called every frame at 60 FPS
# end
# ```
#
# ## Classes
#
# - `Real::Vector2` — 2D vector (x, y)
# - `Real::Window`  — Window and game-loop management (all class methods)
#
# ## Version
#
# ```ruby
# Real::VERSION  # => "0.1"
# ```
module Real
  # The current version of the Real framework.
  VERSION = "0.1"

  # # Real::Vector2
  #
  # A simple 2D vector holding `x` and `y` as `Float` values.
  # Used notably by `Real::Window.size=` to set the window dimensions.
  #
  # ## Creating a vector
  #
  # ```ruby
  # v = Real::Vector2.new(800.0, 600.0)
  # v.x  # => 800.0
  # v.y  # => 600.0
  # ```
  #
  # ## Mutating components
  #
  # ```ruby
  # v.x = 1280.0
  # v.y = 720.0
  # ```
  #
  # ## Notes
  #
  # Both `x` and `y` are stored as C `double` internally.
  # Passing an `Integer` will be coerced to `Float` automatically by mruby.
  class Vector2
    # Creates a new `Vector2`.
    #
    # ```ruby
    # v = Real::Vector2.new(100.0, 200.0)
    # ```
    #
    # @param x [Float] the horizontal component
    #
    # @param y [Float] the vertical component
    def initialize(x, y); end

    # The horizontal component.
    #
    # @return [Float]
    attr_accessor :x

    # The vertical component.
    #
    # @return [Float]
    attr_accessor :y
  end

  # # Real::Window
  #
  # Manages the game window lifecycle and exposes window state queries.
  # All methods are **class-level**; you never instantiate this class.
  #
  # ## Typical usage
  #
  # ```ruby
  # Real::Window.size  = Real::Vector2.new(800.0, 600.0)
  # Real::Window.title = "My Game"
  #
  # Real::Window.open do
  #   # game loop body — runs at 60 FPS
  #   # put your draw calls here
  # end
  # ```
  #
  # Calling `Real::Window.open` **without a block** will open the window
  # but skip frame drawing, resulting in an **infinite loop** with no way
  # to stop it except by killing the process.
  class Window
    # Opens the window and starts the main game loop at **60 FPS**.
    #
    # The provided block is yielded once per frame. The loop exits when
    # the OS close button is clicked or `Real::Window.close` is called.
    #
    # ## Examples
    #
    # Basic window:
    #
    # ```ruby
    # Real::Window.size  = Real::Vector2.new(800.0, 600.0)
    # Real::Window.title = "Hello"
    #
    # Real::Window.open do
    #   # draw calls go here
    # end
    # ```
    #
    # Closing programmatically after 5 seconds:
    #
    # ```ruby
    # start = Time.now
    #
    # Real::Window.open do
    #   Real::Window.close if Time.now - start > 5
    # end
    # ```
    #
    # @yield called once every frame (~16 ms at 60 FPS)
    #
    # @return [self]
    def self.open(&block); end

    # Closes the window immediately.
    # Safe to call even if the window is not open (no-op in that case).
    #
    # @return [self]
    def self.close; end

    # Returns `true` if the window has been initialised and is open.
    #
    # @return [Boolean]
    def self.ready?; end

    # ```ruby
    # Real::Window.title = "My Awesome Game"
    # ```
    #
    # @param title [String] the new window title
    #
    # @return [String] the value that was set
    def self.title=(title); end

    # Returns the current window title.
    # Returns an empty `String` if `title=` was never called.
    #
    # @return [String]
    def self.title; end
  
    # ```ruby
    # Real::Window.opacity = 0.65 # 65% of opacity
    # ```
    # @param opacity [Float] the new opacity
    #
    # @return [self]
    def self.opacity=(opa); end
    
    # Returns the current window opacity.
    #
    # @return [Float]
    def self.opacity; end

    # Returns the current position of the Window in the environment.
    #
    # @return [Real::Vector2]
    def self.position; end

    # Returns the current resolution (width, height) of the Window.
    #
    # @return [Real::Vector2]
    def self.resolution; end

    # Sets the window size.
    #
    # ```ruby
    # Real::Window.size = Real::Vector2.new(1280.0, 720.0)
    # ```
    #
    # @param size [Real::Vector2] desired width and height in pixels
    #
    # @return [Real::Vector2] the value that was set
    #
    # @raise [TypeError] if `size` is not a `Real::Vector2`
    def self.size=(size); end

    # Returns the current screen width reported by raylib (in pixels).
    # This reflects the actual framebuffer width (useful in fullscreen).
    #
    # @return [Integer]
    def self.screen_width; end

    # Returns the current screen height reported by raylib (in pixels).
    # This reflects the actual framebuffer height (useful in fullscreen).
    #
    # @return [Integer]
    def self.screen_height; end

    # Toggles fullscreen mode on/off.
    #
    # @return [self]
    def self.fullscreen; end

    # @return [Boolean] `true` if the window is currently fullscreen
    def self.fullscreen?; end

    # Toggles borderless windowed mode on/off.
    #
    # @return [self]
    def self.borderless; end

    # Minimizes the window to the taskbar.
    #
    # @return [self]
    def self.minimize; end

    # Maximizes the window to fill the screen (non-fullscreen).
    #
    # @return [self]
    def self.maximize; end

    # Restores the window from a minimized or maximized state.
    #
    # @return [self]
    def self.restore; end

    # @return [Boolean] `true` if the window is minimized
    def self.minimized?; end

    # @return [Boolean] `true` if the window is maximized
    def self.maximized?; end

    # @return [Boolean] `true` if the window has input focus
    def self.focused?; end

    # @return [Boolean] `true` if the window was resized this frame
    def self.resized?; end

    # @return [Boolean] `true` if the window is hidden
    def self.hidden?; end
  end
  
  # An interface to the monitor (display).
  #
  # Like Real::Window *you do not need to instantiate it*!
  class Monitor
    
    # Get the refresh rate of the current Monitor.
    #
    # @return [Float]
    def refresh_rate; end

    # Get the width of the current Monitor.
    #
    # @return [Int]
    def width; end

    # Get the height of the current Monitor.
    #
    # @return [Int]
    def height; end

    # Get the physical width of the current Monitor.
    #
    # @return [Int]
    def physical_width; end

    # Get the physical height of the current Monitor.
    #
    # @return [Int]
    def physical_height; end

    # Returns the current resolution (width, height) of the current Monitor.
    #
    # @return [Real::Vector2]
    def self.resolution; end

    # Returns the current physical resolution (physical width, physical height) of the current Monitor.
    #
    # @return [Real::Vector2]
    def self.physical_resolution; end
  end
end

# A **global** method used to stop the process at the OS level for `seconds` seconds.
#
# It exists because mruby does not exposes a sleep method by itself.
def sleep(seconds); end
