/*
    Это упрощенная версия, а полная верси доступна тут: https://github.com/1vanK/Urho3DSpriteBatch .
*/

#pragma once

#include <Urho3D/Urho3DAll.h>

class SpriteBatch : public Object
{
    URHO3D_OBJECT(SpriteBatch, Object);

public:
    SpriteBatch(Context *context);
    virtual ~SpriteBatch();

    // Подготовка к пакетному выводу спрайтов.
    void Begin();

    // Создает спрайт с нужными экранными координатами.
    void Draw(Texture2D* texture, const Vector2& position, const Color& color = Color::WHITE,
        float rotation = 0.0f, const Vector2 &origin = Vector2::ZERO, float scale = 1.0f);

    // Отображает спрайты на экране.
    void End();

private:
    // Отдельный спрайт в очереди на отрисовку.
    struct SBSprite
    {
        Texture2D* texture_;
        Vector2 position_;
        Color color_;

        // Угол поворота спрайта (по часовой стрелке, в градусах).
        float rotation_;

        // Начало координат спрайта (точка отсчета). По умолчанию это левый верхний угол (0, 0).
        // При указании позиции спрайта подразумевается позиция на экране именно этой точки.
        // Вращение спрайта также происходит вокруг этой точки.
        Vector2 origin_;
        
        float scale_;
    };

    // Индексный буфер создается и заполняется один раз, а потом только используется.
    SharedPtr<IndexBuffer> indexBuffer_;

    // Вершинный буфер перезаполняется каждый кадр.
    SharedPtr<VertexBuffer> vertexBuffer_;

    // Спрайты, которые ожидают рендеринга.
    PODVector<SBSprite> sprites_;

    // Кэширование часто используемых вещей.
    Graphics* graphics_;
    ShaderVariation* vs_; // Вершинный шейдер.
    ShaderVariation* ps_; // Пиксельный шейдер.

    // Определяет количество спрайтов, которые можно отрендерить без смены текстуры.
    unsigned GetPortionLength(unsigned start);

    // Рендерит порцию спрайтов, использующих одну и ту же текстуру.
    void RenderPortion(unsigned start, unsigned count);
};
