/*
    Рендерим белый треугольник, используя только вершинный буфер.
*/

#include <Urho3D/Urho3DAll.h>

class Game : public Application
{
    URHO3D_OBJECT(Game, Application);

    Game(Context* context) : Application(context)
    {
        // На момент события E_ENDALLVIEWSRENDER графическая подсистема уже подготовлена
        // к рендерингу нашего треугольника, поэтому подписываемся на это событие.
        SubscribeToEvent(E_ENDALLVIEWSRENDER, URHO3D_HANDLER(Game, HandleEndAllViewsRender));
    }

    // Задаем параметры движка перед инициализацией.
    void Setup()
    {
        engineParameters_[EP_FULL_SCREEN] = false; // Оконный режим.
        engineParameters_[EP_WINDOW_WIDTH] = 800;  // Ширина экрана.
        engineParameters_[EP_WINDOW_HEIGHT] = 600; // Высота экрана.

        // Указываем папки с ресурсами. Обратите внимание, что собственная папка
        // указана первой. Это позволяет подменять стандартные ресурсы движка.
        engineParameters_[EP_RESOURCE_PATHS] = "Step1Data;Data;CoreData";
    }

    // Вершинный буфер.
    SharedPtr<VertexBuffer> vertexBuffer_;

    // В конструкторе и в функции Setup() графическая подсистема еще не проинициализирована.
    // Поэтому создаем вершинный буфер тут.
    void Start()
    {
        vertexBuffer_ = new VertexBuffer(context_);

        // Включаем дублирование буфера в памяти CPU. Это позволяет автоматически
        // восстанавливать данные в случае необходимости.
        vertexBuffer_->SetShadowed(true);

        // Задаем число и формат вершин.
#if true
        // Старый способ: задаем битовую маску.
        vertexBuffer_->SetSize(3, MASK_POSITION);
#else
        // Более новый способ: создаем список атрибутов. Смотрите также пример 34_DynamicGeometry.
        PODVector<VertexElement> elements;
        elements.Push(VertexElement(TYPE_VECTOR3, SEM_POSITION));
        vertexBuffer_->SetSize(3, elements);
#endif

        // Блокируем диапазон.
        float* data = (float*)vertexBuffer_->Lock(0, vertexBuffer_->GetVertexCount());

        // Первая вершина в центре экрана.
        data[0] = 0.0f; // x
        data[1] = 0.0f; // y
        data[2] = 0.0f; // z

        // Вторая вершина справа.
        data[3] = 0.5f; // x
        data[4] = 0.0f; // y
        data[5] = 0.0f; // z

        // Третья вершина справа внизу.
        data[6] = 0.5f;  // x
        data[7] = -0.5f; // y
        data[8] = 0.0f;  // z

        vertexBuffer_->Unlock();
    }

    // Функция выполняется каждый кадр.
    void HandleEndAllViewsRender(StringHash eventType, VariantMap& eventData)
    {
        Graphics* graphics = GetSubsystem<Graphics>();

        // Получаем вершинный и пиксельный шейдеры. Если запрашиваемый шейдер уже был
        // скомпилирован ранее, он не будет компилироваться повторно.
        ShaderVariation* vs = graphics->GetShader(VS, "White");
        ShaderVariation* ps = graphics->GetShader(PS, "White");

        // Устанавливаем текущий вершинный буфер.
        graphics->SetVertexBuffer(vertexBuffer_);
        // Указываем текущие шейдеры.
        graphics->SetShaders(vs, ps);

        // Передаем единичные матрицы в шейдерную программу. Вершины треугольника не будут
        // трансформированы. Обратите внимание, что матрицы разной размерности.
        graphics->SetShaderParameter(VSP_MODEL, Matrix3x4::IDENTITY);
        graphics->SetShaderParameter(VSP_VIEWPROJ, Matrix4::IDENTITY);

        // И, наконец, рендерим наш треугольник.
        graphics->Draw(TRIANGLE_LIST, 0, vertexBuffer_->GetVertexCount());
    }
};

URHO3D_DEFINE_APPLICATION_MAIN(Game)
