#include "SpriteBatch.h"

// Спрайт состоит из двух треугольников, а значит у него 6 вершин.
// То есть каждый спрайт занимает 6 элементов в индексном буфере.
#define INDICES_PER_SPRITE 6

// Две вершины спрайта идентичны для обоих треугольников, поэтому
// в вершинном буфере каждый спрайт занимает 4 элемента.
#define VERTICES_PER_SPRITE 4

// Максимальное число выводимых за один Draw Call спрайтов.
// В полной версии https://github.com/1vanK/Urho3DSpriteBatch
// используется более корректное значение.
#define MAX_PORTION_SIZE 2000

// Атрибуты вершин.
struct SBVertex
{
    Vector3 position_;
    unsigned color_;
    Vector2 uv_;
};

SpriteBatch::SpriteBatch(Context *context) : Object(context)
{
    // Индексный буфер никогда не меняется, поэтому мы можем его сразу заполнить.
    indexBuffer_ = new IndexBuffer(context_);
    indexBuffer_->SetShadowed(true);
    indexBuffer_->SetSize(MAX_PORTION_SIZE * INDICES_PER_SPRITE, false);
    unsigned short* buffer = (unsigned short*)indexBuffer_->Lock(0, indexBuffer_->GetIndexCount());
    for (unsigned i = 0; i < MAX_PORTION_SIZE; i++)
    {
        // Первый треугольник спрайта.
        buffer[i * INDICES_PER_SPRITE + 0] = i * VERTICES_PER_SPRITE + 0;
        buffer[i * INDICES_PER_SPRITE + 1] = i * VERTICES_PER_SPRITE + 1;
        buffer[i * INDICES_PER_SPRITE + 2] = i * VERTICES_PER_SPRITE + 2;
        
        // Второй треугольник спрайта.
        buffer[i * INDICES_PER_SPRITE + 3] = i * VERTICES_PER_SPRITE + 2;
        buffer[i * INDICES_PER_SPRITE + 4] = i * VERTICES_PER_SPRITE + 3;
        buffer[i * INDICES_PER_SPRITE + 5] = i * VERTICES_PER_SPRITE + 0;
    }
    indexBuffer_->Unlock();

    vertexBuffer_ = new VertexBuffer(context_);
    vertexBuffer_->SetSize(MAX_PORTION_SIZE * VERTICES_PER_SPRITE,
                           MASK_POSITION | MASK_COLOR | MASK_TEXCOORD1, true);

    // Немного ускоряем доступ к подсистеме.
    graphics_ = GetSubsystem<Graphics>();

    // Используем стандартный шейдер.
    vs_ = graphics_->GetShader(VS, "Basic", "DIFFMAP VERTEXCOLOR");
    ps_ = graphics_->GetShader(PS, "Basic", "DIFFMAP VERTEXCOLOR");
}

SpriteBatch::~SpriteBatch()
{
}

void SpriteBatch::Begin()
{
    // Очищаем старый список спрайтов.
    sprites_.Clear();
}

void SpriteBatch::Draw(Texture2D* texture, const Vector2& position, const Color& color/* = Color::WHITE*/,
    float rotation/* = 0.0f*/, const Vector2 &origin/* = Vector2::ZERO*/, float scale/* = 1.0f*/)
{
    // Просто добавляем очередной спрайт в список.
    SBSprite sprite { texture, position, color, rotation, origin, scale };
    sprites_.Push(sprite);
}

void SpriteBatch::End()
{
    // Список спрайтов пуст.
    if (sprites_.Size() == 0)
        return;

    // Включаем альфа-смешивание.
    graphics_->SetBlendMode(BLEND_ALPHA);

    // Устанавливаем текущие буферы.
    graphics_->SetVertexBuffer(vertexBuffer_);
    graphics_->SetIndexBuffer(indexBuffer_);

    // Устанавливаем используемую шейдерную программу.
    graphics_->SetShaders(vs_, ps_);

    // Шейдер Basic требует это значение. Информацию о цвете спрайта мы храним
    // в вершинах, поэтому здесь просто белый цвет.
    graphics_->SetShaderParameter(PSP_MATDIFFCOLOR, Color::WHITE);

    // Матрица модели не используется. Локальные и мировые координаты совпадают.
    graphics_->SetShaderParameter(VSP_MODEL, Matrix3x4::IDENTITY);

    // Фрагменты спрайтов должны совпадать с пикселями на экране (вершины спрайтов
    // позиционируются между пикселями). Но изначально экранные координаты находятся
    // в диапазоне [-1, 1] по вертикали и горизонтали (начало координат в центре экрана).
    // И нам нужно привести их к диапазону [0, screenWidth] по горизонтали и [0, screenHeight]
    // по вертикали. То есть каждую вершину при рендеринге необходимо отмасштабировать
    // и сдвинуть. Заодно нужно поменять направление оси Y на противоположное (направить ее вниз).
    // В матрице присутствует двойка, так как длина отрезка [-1, 1] равна 2.
    // Как составляются подобные матрицы показано в функции RenderPortion().
    float w = (float)graphics_->GetWidth();
    float h = (float)graphics_->GetHeight();
    Matrix4 viewProjMatrix = Matrix4(2.0f / w, 0.0f,     0.0f, -1.0f,
                                     0.0f,    -2.0f / h, 0.0f,  1.0f,
                                     0.0f,     0.0f,     1.0f,  0.0f,
                                     0.0f,     0.0f,     0.0f,  1.0f);
    graphics_->SetShaderParameter(VSP_VIEWPROJ, viewProjMatrix);

    // Индекс, с которого начинается очередная порция спрайтов.
    unsigned startSpriteIndex = 0;
    
    // Выполняем, пока все спрайты из списка не будут отрисованы.
    while (startSpriteIndex != sprites_.Size())
    {
        // Определяем число спрайтов с одинаковой текстурой.
        unsigned count = GetPortionLength(startSpriteIndex);

        // Рендерим очередную порцию.
        RenderPortion(startSpriteIndex, count);

        startSpriteIndex += count;
    }
}

unsigned SpriteBatch::GetPortionLength(unsigned start)
{
    unsigned count = 1;

    while (true)
    {
        // Порция уже максимального размера.
        if (count >= MAX_PORTION_SIZE)
            break;

        // Индекс следующего спрайта.
        unsigned nextSpriteIndex = start + count;
        
        // Достигнут конец списка.
        if (nextSpriteIndex == sprites_.Size())
            break;
        
        // У следующего спрайта другая текстура.
        if (sprites_[nextSpriteIndex].texture_ != sprites_[start].texture_)
            break;

        count++;
    }

    return count;
}

void SpriteBatch::RenderPortion(unsigned start, unsigned count)
{
    // Текстура для данной порции спрайтов.
    Texture2D* texture = sprites_[start].texture_;
    float w = (float)texture->GetWidth();
    float h = (float)texture->GetHeight();

    // Начинаем заполнение вершинного буфера.
    SBVertex* vertices = (SBVertex*)vertexBuffer_->Lock(0, count * VERTICES_PER_SPRITE, true);
    
    // Цикл для всех спрайтов порции.
    for (unsigned i = 0; i < count; i++)
    {
        // Очередной спрайт.
        SBSprite* sprite = sprites_.Buffer() + start + i;

        // Для краткости.
        unsigned color = sprite->color_.ToUInt();
        Vector2 pos    = sprite->position_;
        Vector2 origin = sprite->origin_;
        float scale    = sprite->scale_;
        float rotation = sprite->rotation_;

        // Если спрайт не повернут, то прорисовка очень проста.
        if (rotation == 0.0f && scale == 1.0f)
        {
            // Сдвигаем спрайт на -origin.
            pos -= origin;

            // Лицевая грань задается по часовой стрелке. Учитываем, что для экрана ось Y направлена вниз.
            vertices[i * VERTICES_PER_SPRITE + 0].position_ = Vector3(pos.x_,     pos.y_,     0.0f);
            vertices[i * VERTICES_PER_SPRITE + 1].position_ = Vector3(pos.x_ + w, pos.y_,     0.0f);
            vertices[i * VERTICES_PER_SPRITE + 2].position_ = Vector3(pos.x_ + w, pos.y_ + h, 0.0f);
            vertices[i * VERTICES_PER_SPRITE + 3].position_ = Vector3(pos.x_,     pos.y_ + h, 0.0f);
        }
        else
        {
            // Вычисления производятся в однородных координатах, поэтому к 2D-координатам
            // добавляется третье число: http://opengl-tutorial.blogspot.ru/p/3.html
            Vector3 v0 = Vector3(0.0f, 0.0f, 1.0f); // Верхний левый угол спрайта.
            Vector3 v1 = Vector3(w,    0.0f, 1.0f); // Правый верхний угол.
            Vector3 v2 = Vector3(w,    h,    1.0f); // Нижний правый угол.
            Vector3 v3 = Vector3(0.0f, h,    1.0f); // Левый нижний угол.

            // Трансформация = перемещение * вращение * масштабирование.
            // Почему такой порядок умножений смотрите здесь:
            // https://github.com/1vanK/Urho3DRuWiki/wiki/Памятка-о-матрицах
            // |1  0  dx|   |cos -sin  0|   |s  0  0|   |cos*s -sin*s  dx|
            // |0  1  dy| * |sin  cos  0| * |0  s  0| = |sin*s  cos*s  dy|
            // |0  0  1 |   |0    0    1|   |0  0  1|   |0      0      1 |
            // Так как есть origin, то сперва нужно сместить на -origin.
            // |cos*s -sin*s  dx|   |1  0 -ox|   |cos*s  -sin*s  -ox*cos*s + oy*sin*s + dx|
            // |sin*s  cos*s  dy| * |0  1 -oy| = |sin*s   cos*s  -ox*sin*s - oy*cos*s + dy|
            // |0      0      1 |   |0  0  1 |   |0       0       1                       |

            float sin, cos;
            SinCos(rotation, sin, cos);

            Matrix3 transform
            {
                cos*scale, -sin*scale, -origin.x_*cos*scale + origin.y_*sin*scale + pos.x_,
                sin*scale,  cos*scale, -origin.x_*sin*scale - origin.y_*cos*scale + pos.y_,
                0.0f,       0.0f,       1.0f
            };
            
            v0 = transform * v0;
            // У нас 2D-координаты хранятся в 3D-векторе. И в атрибут вершины как раз нужно
            // записать 3D-вектор. Поэтому можно не копировать координаты в новый вектор,
            // а просто установить нужное значение глубины (1.0f меняется на 0.0f).
            v0.z_ = 0.0f;
            vertices[i * VERTICES_PER_SPRITE + 0].position_ = v0;

            // То же самое для других вершин.
            v1 = transform * v1;
            v1.z_ = 0.0f;
            vertices[i * VERTICES_PER_SPRITE + 1].position_ = v1;

            v2 = transform * v2;
            v2.z_ = 0.0f;
            vertices[i * VERTICES_PER_SPRITE + 2].position_ = v2;

            v3 = transform * v3;
            v3.z_ = 0.0f;
            vertices[i * VERTICES_PER_SPRITE + 3].position_ = v3;
        }

        // Цвет вершин.
        vertices[i * VERTICES_PER_SPRITE + 0].color_ = color;
        vertices[i * VERTICES_PER_SPRITE + 1].color_ = color;
        vertices[i * VERTICES_PER_SPRITE + 2].color_ = color;
        vertices[i * VERTICES_PER_SPRITE + 3].color_ = color;

        // Текстурные координаты.
        vertices[i * VERTICES_PER_SPRITE + 0].uv_ = Vector2(0.0f, 0.0f);
        vertices[i * VERTICES_PER_SPRITE + 1].uv_ = Vector2(1.0f, 0.0f);
        vertices[i * VERTICES_PER_SPRITE + 2].uv_ = Vector2(1.0f, 1.0f);
        vertices[i * VERTICES_PER_SPRITE + 3].uv_ = Vector2(0.0f, 1.0f);
    }

    vertexBuffer_->Unlock();
    
    graphics_->SetTexture(TU_DIFFUSE, texture);
    graphics_->Draw(TRIANGLE_LIST, 0, count * INDICES_PER_SPRITE, 0, count * VERTICES_PER_SPRITE);
}
