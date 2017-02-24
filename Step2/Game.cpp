/*
    Рендерим градиентный треугольник и текстурированные квадраты,
    используя вершинный и индексные буферы.
*/

#include <Urho3D/Urho3DAll.h>

class Game : public Application
{
    URHO3D_OBJECT(Game, Application);

    Game(Context* context) : Application(context)
    {
        SubscribeToEvent(E_ENDALLVIEWSRENDER, URHO3D_HANDLER(Game, HandleEndAllViewsRender));
    }

    void Setup()
    {
        engineParameters_[EP_FULL_SCREEN] = false;
        engineParameters_[EP_WINDOW_WIDTH] = 800;
        engineParameters_[EP_WINDOW_HEIGHT] = 600;
        engineParameters_[EP_RESOURCE_PATHS] = "Step2Data;Data;CoreData";
    }

    // Общий вершинный буфер для обеих геометрических фигур.
    SharedPtr<VertexBuffer> vertexBuffer_;

    // Индексные буферы для треугольника и прямоугольника.
    SharedPtr<IndexBuffer> triangleIndexBuffer_;
    SharedPtr<IndexBuffer> rectangleIndexBuffer_;

    void Start()
    {
        vertexBuffer_ = new VertexBuffer(context_);
        vertexBuffer_->SetShadowed(true);
  
        // Три вершины треугольника и четыре вершины прямоугольника.
        vertexBuffer_->SetSize(3 + 4, MASK_POSITION | MASK_COLOR | MASK_TEXCOORD1);

        float* vertexData = (float*)vertexBuffer_->Lock(0, vertexBuffer_->GetVertexCount());
        int i = 0;

        // Первая вершина треугольника (слева вверху).
        vertexData[i++] = -0.5f; // x
        vertexData[i++] = 0.5f;  // y
        vertexData[i++] = 0.0f;  // z
        ((unsigned&)vertexData[i++]) = Color::RED.ToUInt(); // цвет
        vertexData[i++] = 0.0f; // u (горизонтальная текстурная координата, обычно находится в диапазоне 0.0 - 1.0)
        vertexData[i++] = 0.0f; // v (вертикальная текстурная координата)

        // Вторая вершина треугольника (выше центра экрана).
        vertexData[i++] = 0.0f; // x
        vertexData[i++] = 0.5f; // y
        vertexData[i++] = 0.0f; // z
        ((unsigned&)vertexData[i++]) = Color::GREEN.ToUInt(); // цвет
        vertexData[i++] = 1.0f; // u
        vertexData[i++] = 0.0f; // v

        // Третья вершина треугольника (в центре экрана).
        vertexData[i++] = 0.0f; // x
        vertexData[i++] = 0.0f; // y
        vertexData[i++] = 0.0f; // z
        ((unsigned&)vertexData[i++]) = Color::BLUE.ToUInt(); // цвет
        vertexData[i++] = 1.0f; // u
        vertexData[i++] = 1.0f; // v

        // Первая вершина прямоугольника (слева вверху).
        vertexData[i++] = -0.25f; // x
        vertexData[i++] = 0.25f;  // y
        vertexData[i++] = 0.0f;   // z
        ((unsigned&)vertexData[i++]) = Color::MAGENTA.ToUInt(); // цвет
        vertexData[i++] = 0.0f; // u
        vertexData[i++] = 0.0f; // v

        // Вторая вершина прямоугольника (справа вверху).
        vertexData[i++] = 0.25f; // x
        vertexData[i++] = 0.25f; // y
        vertexData[i++] = 0.0f;  // z
        ((unsigned&)vertexData[i++]) = Color::MAGENTA.ToUInt(); // цвет
        vertexData[i++] = 1.0f; // u
        vertexData[i++] = 0.0f; // v

        // Третья вершина прямоугольника (справа внизу).
        vertexData[i++] = 0.25f;  // x
        vertexData[i++] = -0.25f; // y
        vertexData[i++] = 0.0f;   // z
        ((unsigned&)vertexData[i++]) = Color::MAGENTA.ToUInt(); // цвет
        vertexData[i++] = 1.0f; // u
        vertexData[i++] = 1.0f; // v

        // Четвертая вершина прямоугольника (слева внизу).
        vertexData[i++] = -0.25f; // x
        vertexData[i++] = -0.25f; // y
        vertexData[i++] = 0.0f;   // z
        ((unsigned&)vertexData[i++]) = Color::MAGENTA.ToUInt(); // цвет
        vertexData[i++] = 0.0f; // u
        vertexData[i++] = 1.0f; // v

        vertexBuffer_->Unlock();

        // Индексный буфер для треугольника.
        triangleIndexBuffer_ = new IndexBuffer(context_);
        triangleIndexBuffer_->SetShadowed(true);
        triangleIndexBuffer_->SetSize(3, false); // Используем 16-ти битные индексы.

        unsigned short* triangleIndexData =
            (unsigned short*)triangleIndexBuffer_->Lock(0, triangleIndexBuffer_->GetIndexCount());

        triangleIndexData[0] = 0;
        triangleIndexData[1] = 1;
        triangleIndexData[2] = 2;

        triangleIndexBuffer_->Unlock();

        // Индексный буфер для квадрата.
        rectangleIndexBuffer_ = new IndexBuffer(context_);
        rectangleIndexBuffer_->SetShadowed(true);
        rectangleIndexBuffer_->SetSize(6, false); // Квадрат состоит из 2-х треугольников (6 вершин).

        unsigned short* rectangleIndexData =
            (unsigned short*)rectangleIndexBuffer_->Lock(0, rectangleIndexBuffer_->GetIndexCount());

        // Квадрат состоит из двух треугольников:
        // 0 ____ 1
        //  |\   |
        //  | \  |
        //  |  \ |
        //  |___\|
        // 3      2
        // Первые три вершины в вершинном буфере не относятся к квадрату, поэтому везде прибавляется 3.
        rectangleIndexData[0] = 3 + 0;
        rectangleIndexData[1] = 3 + 1;
        rectangleIndexData[2] = 3 + 2;

        rectangleIndexData[3] = 3 + 2;
        rectangleIndexData[4] = 3 + 3;
        rectangleIndexData[5] = 3 + 0;

        rectangleIndexBuffer_->Unlock();
    }

    void HandleEndAllViewsRender(StringHash eventType, VariantMap& eventData)
    {
        Graphics* graphics = GetSubsystem<Graphics>();
        ResourceCache* cache = GetSubsystem<ResourceCache>();

        // Получаем различные вариации убершейдера.

        // Шейдер закрашивает геометрию белым цветом (в данном примере не используется).
        ShaderVariation* vs = graphics->GetShader(VS, "MyUberShader");
        ShaderVariation* ps = graphics->GetShader(PS, "MyUberShader");

        // Шейдер учитывает цвет вершин.
        ShaderVariation* vsVCol = graphics->GetShader(VS, "MyUberShader", "VERTEXCOLOR");
        ShaderVariation* psVCol = graphics->GetShader(PS, "MyUberShader", "VERTEXCOLOR");

        // Шейдер накладывает текстуру.
        ShaderVariation* vsDiff = graphics->GetShader(VS, "MyUberShader", "DIFFMAP");
        ShaderVariation* psDiff = graphics->GetShader(PS, "MyUberShader", "DIFFMAP");

        // Шейдер накладывает текстуру и учитывает цвет вершин.
        ShaderVariation* vsDiffVCol = graphics->GetShader(VS, "MyUberShader", "DIFFMAP VERTEXCOLOR");
        ShaderVariation* psDiffVCol = graphics->GetShader(PS, "MyUberShader", "DIFFMAP VERTEXCOLOR");

        // Буфер глубины не используется, фигуры накладываются в порядке отрисовки.
        graphics->SetDepthTest(CMP_ALWAYS);
        graphics->SetDepthWrite(false);

        graphics->SetVertexBuffer(vertexBuffer_);
        graphics->SetTexture(TU_DIFFUSE, cache->GetResource<Texture2D>("Textures/UrhoIcon.png"));

        // Рисуем затекстуренный квадрат (который выглядит как прямоугольник).
        graphics->SetIndexBuffer(rectangleIndexBuffer_);
        graphics->SetShaders(vsDiff, psDiff);
        graphics->SetShaderParameter(VSP_MODEL, Matrix3x4::IDENTITY);
        graphics->SetShaderParameter(VSP_VIEWPROJ, Matrix4::IDENTITY);
        graphics->Draw(TRIANGLE_LIST, 0, 6, 3, 4);

        // Рисуем градиентный треугольник.
        graphics->SetIndexBuffer(triangleIndexBuffer_);
        graphics->SetShaders(vsVCol, psVCol);
        graphics->SetShaderParameter(VSP_MODEL, Matrix3x4::IDENTITY);
        graphics->SetShaderParameter(VSP_VIEWPROJ, Matrix4::IDENTITY);
        graphics->Draw(TRIANGLE_LIST, 0, 3, 0, 3);

        // Меняем координаты движущегося квадрата по гармоническому закону в зависимости от времени.
        float time = GetSubsystem<Time>()->GetElapsedTime(); // Время с запуска программы.
        float x, y;
        SinCos(time * 100.0f, x, y);
        Vector3 rectanglePos = Vector3(x * 0.5f, y * 0.5f, 0.0f);

        // Сжимаем квадрат по горизонтали, чтобы компенсировать неквадратность окна.
        Vector3 rectScale = Vector3(graphics->GetHeight() / (float)graphics->GetWidth(), 1.0f, 1.0f);

        // Рисуем движущийся квадрат.
        graphics->SetIndexBuffer(rectangleIndexBuffer_);
        graphics->SetShaders(vsDiffVCol, psDiffVCol);
        graphics->SetShaderParameter(VSP_MODEL, Matrix3x4(rectanglePos, Quaternion::IDENTITY, rectScale));
        graphics->SetShaderParameter(VSP_VIEWPROJ, Matrix4::IDENTITY);
        graphics->Draw(TRIANGLE_LIST, 0, 6, 3, 4);
    }
};

URHO3D_DEFINE_APPLICATION_MAIN(Game)
