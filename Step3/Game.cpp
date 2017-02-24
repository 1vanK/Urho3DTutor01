#include <Urho3D/Urho3DAll.h>
#include "SpriteBatch.h"

class Game : public Application
{
    URHO3D_OBJECT(Game, Application);

public:
    // Пачка спрайтов.
    SharedPtr<SpriteBatch> spriteBatch_;

    Game(Context* context) : Application(context)
    {
    }

    void Setup()
    {
        engineParameters_[EP_FULL_SCREEN] = false;
        engineParameters_[EP_WINDOW_WIDTH] = 800;
        engineParameters_[EP_WINDOW_HEIGHT] = 600;
    }

    Vector2 ballPos_; // Текущее положение мяча.
    Vector2 ballSpeed_ = Vector2(250.0f, 150.0f); // Текущая скорость мяча.
    float ballAngle_ = 0.0f; // Текущий угол поворота мяча.

    void Start()
    {
        spriteBatch_ = new SpriteBatch(context_);

        // Инициализируем генератор случайных чисел текущим временем,
        // чтобы при каждом запуске программы он выдавал уникальную последовательность.
        SetRandomSeed(Time::GetSystemTime());
        // Случайное положение мяча на экране.
        ballPos_ = Vector2(Random(200.0f, 600.0f), Random(200.0f, 400.0f));

        // Данный режим предназначен для самостоятельного рендеринга курсора мыши.
        GetSubsystem<Input>()->SetMouseMode(MM_FREE);

        // Задаем цвет фона окна.
        GetSubsystem<Renderer>()->GetDefaultZone()->SetFogColor(Color(0.4f, 0.5f, 0.8f));

        // Создаем отладочный худ.
        XMLFile* xmlFile = GetSubsystem<ResourceCache>()->GetResource<XMLFile>("UI/DefaultStyle.xml");
        DebugHud* debugHud = engine_->CreateDebugHud();
        debugHud->SetDefaultStyle(xmlFile);

        SubscribeToEvent(E_UPDATE, URHO3D_HANDLER(Game, HandleUpdate));
        SubscribeToEvent(E_ENDALLVIEWSRENDER, URHO3D_HANDLER(Game, HandleEndAllViewsRender));
    }

    // Нода для источника звука. Не принадлежит ни одной сцене.
    SharedPtr<Node> soundNode_;
    
    // Проигрывает звук.
    void PlaySound(const String& fileName)
    {
        // Создаем ноду, если еще не создана.
        if (!soundNode_)
            soundNode_ = new Node(context_);

        SoundSource* soundSource = soundNode_->GetOrCreateComponent<SoundSource>();

        if (!soundSource->IsPlaying())
        {
            Sound* sound = GetSubsystem<ResourceCache>()->GetResource<Sound>(fileName);
            soundSource->Play(sound);
        }
    }

    void HandleUpdate(StringHash eventType, VariantMap& eventData)
    {
        using namespace Update;
        float timeStep = eventData[P_TIMESTEP].GetFloat();

        // Показываем / прячем отладочный худ при нажатии на F2.
        if (GetSubsystem<Input>()->GetKeyPress(KEY_F2))
            GetSubsystem<DebugHud>()->ToggleAll();

        // Вращаем спрайт мяча.
        ballAngle_ += timeStep * 100.0f;
        while (ballAngle_ >= 360.0f)
            ballAngle_ -= 360.0f;

        // Перемещаем мяч.
        ballPos_ += ballSpeed_ * timeStep;

        // Не позволяем мячу улетать за границы экрана.
        
        bool wasBounce = false; // Нужно ли проигрывать звук отскока мяча от края экрана.
        const float BALL_RADIUS = 16.0f; // Радиус мяча в пикселях.
        
        if (ballPos_.x_ <= BALL_RADIUS && ballSpeed_.x_ < 0.0f)
        {
            ballSpeed_.x_ = Abs(ballSpeed_.x_);
            wasBounce = true;
        }
        
        if (ballPos_.x_ >= 800.0f - BALL_RADIUS && ballSpeed_.x_ > 0.0f)
        {
            ballSpeed_.x_ = -Abs(ballSpeed_.x_);
            wasBounce = true;
        }

        if (ballPos_.y_ <= BALL_RADIUS && ballSpeed_.y_ < 0.0f)
        {
            ballSpeed_.y_ = Abs(ballSpeed_.y_);
            wasBounce = true;
        }

        if (ballPos_.y_ >= 600.0f - BALL_RADIUS && ballSpeed_.y_ > 0.0f)
        {
            ballSpeed_.y_ = -Abs(ballSpeed_.y_);
            wasBounce = true;
        }

        // Проигрываем звук отскока.
        if (wasBounce)
            PlaySound("Data/Sounds/PlayerFistHit.wav");
    }

    void HandleEndAllViewsRender(StringHash eventType, VariantMap& eventData)
    {
        ResourceCache* cache = GetSubsystem<ResourceCache>();
        Texture2D* ball = cache->GetResource<Texture2D>("Urho2D/Ball.png");
        Texture2D* cursor = cache->GetResource<Texture2D>("Urho2D/greenspiral.png");

        // Можно не задавать цвет зоны, а очищать экран так. Но это будет немного медленнее,
        // так как происходит повторная очистка.
        //GetSubsystem<Graphics>()->Clear(CLEAR_COLOR, Color::BLUE);

        spriteBatch_->Begin();

        // Рисуем мяч.
        spriteBatch_->Draw(ball, ballPos_, Color::WHITE, ballAngle_, Vector2(16.0f, 16.0f));

        // Рисуем курсор.
        Vector2 cursorPos = Vector2(GetSubsystem<Input>()->GetMousePosition());
        // Масштаб курсора меняется в диапазоне [0.5, 1.0].
        float cursorScale = Cos(GetSubsystem<Time>()->GetElapsedTime() * 100.0f) * 0.25f + 0.75f;
        spriteBatch_->Draw(cursor, cursorPos, Color::BLACK, 0.0f, Vector2(16.0f, 16.0f), cursorScale);

        spriteBatch_->End();
    }
};

URHO3D_DEFINE_APPLICATION_MAIN(Game)
