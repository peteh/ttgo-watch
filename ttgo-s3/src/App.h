class App
{
public:
    virtual void setup() = 0;
    virtual void loop() = 0;
    virtual ~App() = default; // always add this
};