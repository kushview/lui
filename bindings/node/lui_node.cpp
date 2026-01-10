// Copyright 2026 Michael Fisher <mfisher@lvtk.org>
// SPDX-License-Identifier: ISC

#include <napi.h>
#include <lui/main.hpp>
#include <lui/widget.hpp>
#include <lui/button.hpp>
#include <lui/slider.hpp>
#include <lui/opengl.hpp>
#include <memory>
#include <iostream>
#include <map>

namespace lui_node {

// Forward declarations
class WidgetWrap;
class MainWrap;

// Global storage for persistent objects
static std::unique_ptr<lui::Main> g_main;
static std::map<lui::Widget*, Napi::ObjectReference*> g_widget_refs;

// ============================================================================
// Widget Wrapper
// ============================================================================
class WidgetWrap : public Napi::ObjectWrap<WidgetWrap> {
public:
    static Napi::Object Init(Napi::Env env, Napi::Object exports) {
        Napi::Function func = DefineClass(env, "Widget", {
            InstanceMethod("setName", &WidgetWrap::SetName),
            InstanceMethod("getName", &WidgetWrap::GetName),
            InstanceMethod("add", &WidgetWrap::Add),
            InstanceMethod("remove", &WidgetWrap::Remove),
            InstanceMethod("setVisible", &WidgetWrap::SetVisible),
            InstanceMethod("visible", &WidgetWrap::Visible),
            InstanceMethod("setBounds", &WidgetWrap::SetBounds),
            InstanceMethod("setSize", &WidgetWrap::SetSize),
            InstanceMethod("bounds", &WidgetWrap::GetBounds),
            InstanceMethod("repaint", &WidgetWrap::Repaint),
            InstanceMethod("setOpaque", &WidgetWrap::SetOpaque),
        });

        exports.Set("Widget", func);
        return exports;
    }

    WidgetWrap(const Napi::CallbackInfo& info) 
        : Napi::ObjectWrap<WidgetWrap>(info) {
        widget_ = std::make_unique<lui::Widget>();
        g_widget_refs[widget_.get()] = new Napi::ObjectReference();
        g_widget_refs[widget_.get()]->Reset(info.This().As<Napi::Object>(), 1);
    }

    ~WidgetWrap() {
        if (widget_) {
            auto it = g_widget_refs.find(widget_.get());
            if (it != g_widget_refs.end()) {
                delete it->second;
                g_widget_refs.erase(it);
            }
        }
    }

    lui::Widget* GetWidget() { return widget_.get(); }

private:
    Napi::Value SetName(const Napi::CallbackInfo& info) {
        if (info.Length() < 1 || !info[0].IsString()) {
            Napi::TypeError::New(info.Env(), "String expected").ThrowAsJavaScriptException();
            return info.Env().Undefined();
        }
        widget_->set_name(info[0].As<Napi::String>().Utf8Value());
        return info.This();
    }

    Napi::Value GetName(const Napi::CallbackInfo& info) {
        return Napi::String::New(info.Env(), widget_->name());
    }

    Napi::Value Add(const Napi::CallbackInfo& info) {
        if (info.Length() < 1 || !info[0].IsObject()) {
            Napi::TypeError::New(info.Env(), "Widget expected").ThrowAsJavaScriptException();
            return info.Env().Undefined();
        }
        
        auto other = Napi::ObjectWrap<WidgetWrap>::Unwrap(info[0].As<Napi::Object>());
        if (other && other->GetWidget()) {
            widget_->add(*other->GetWidget());
        }
        return info.This();
    }

    Napi::Value Remove(const Napi::CallbackInfo& info) {
        if (info.Length() < 1 || !info[0].IsObject()) {
            Napi::TypeError::New(info.Env(), "Widget expected").ThrowAsJavaScriptException();
            return info.Env().Undefined();
        }
        
        auto other = Napi::ObjectWrap<WidgetWrap>::Unwrap(info[0].As<Napi::Object>());
        if (other && other->GetWidget()) {
            widget_->remove(*other->GetWidget());
        }
        return info.This();
    }

    Napi::Value SetVisible(const Napi::CallbackInfo& info) {
        if (info.Length() < 1 || !info[0].IsBoolean()) {
            Napi::TypeError::New(info.Env(), "Boolean expected").ThrowAsJavaScriptException();
            return info.Env().Undefined();
        }
        widget_->set_visible(info[0].As<Napi::Boolean>().Value());
        return info.This();
    }

    Napi::Value Visible(const Napi::CallbackInfo& info) {
        return Napi::Boolean::New(info.Env(), widget_->visible());
    }

    Napi::Value SetBounds(const Napi::CallbackInfo& info) {
        if (info.Length() < 4) {
            Napi::TypeError::New(info.Env(), "4 numbers expected (x, y, width, height)").ThrowAsJavaScriptException();
            return info.Env().Undefined();
        }
        widget_->set_bounds(
            info[0].As<Napi::Number>().Int32Value(),
            info[1].As<Napi::Number>().Int32Value(),
            info[2].As<Napi::Number>().Int32Value(),
            info[3].As<Napi::Number>().Int32Value()
        );
        return info.This();
    }

    Napi::Value SetSize(const Napi::CallbackInfo& info) {
        if (info.Length() < 2) {
            Napi::TypeError::New(info.Env(), "2 numbers expected (width, height)").ThrowAsJavaScriptException();
            return info.Env().Undefined();
        }
        widget_->set_size(
            info[0].As<Napi::Number>().Int32Value(),
            info[1].As<Napi::Number>().Int32Value()
        );
        return info.This();
    }

    Napi::Value GetBounds(const Napi::CallbackInfo& info) {
        auto b = widget_->bounds();
        auto obj = Napi::Object::New(info.Env());
        obj.Set("x", Napi::Number::New(info.Env(), b.x));
        obj.Set("y", Napi::Number::New(info.Env(), b.y));
        obj.Set("width", Napi::Number::New(info.Env(), b.width));
        obj.Set("height", Napi::Number::New(info.Env(), b.height));
        return obj;
    }

    Napi::Value Repaint(const Napi::CallbackInfo& info) {
        widget_->repaint();
        return info.This();
    }

    Napi::Value SetOpaque(const Napi::CallbackInfo& info) {
        if (info.Length() < 1 || !info[0].IsBoolean()) {
            Napi::TypeError::New(info.Env(), "Boolean expected").ThrowAsJavaScriptException();
            return info.Env().Undefined();
        }
        // set_opaque is protected, skip for now
        return info.This();
    }

    std::unique_ptr<lui::Widget> widget_;
};

// ============================================================================
// Button Wrapper
// ============================================================================
class ButtonWrap : public Napi::ObjectWrap<ButtonWrap> {
public:
    static Napi::Object Init(Napi::Env env, Napi::Object exports) {
        Napi::Function func = DefineClass(env, "Button", {
            InstanceMethod("setName", &ButtonWrap::SetName),
            InstanceMethod("getName", &ButtonWrap::GetName),
            InstanceMethod("setText", &ButtonWrap::SetText),
            InstanceMethod("getText", &ButtonWrap::GetText),
            InstanceMethod("setBounds", &ButtonWrap::SetBounds),
            InstanceMethod("setVisible", &ButtonWrap::SetVisible),
            InstanceMethod("onClick", &ButtonWrap::OnClick),
        });

        exports.Set("Button", func);
        return exports;
    }

    ButtonWrap(const Napi::CallbackInfo& info) 
        : Napi::ObjectWrap<ButtonWrap>(info) {
        button_ = std::make_unique<lui::TextButton>();
        g_widget_refs[button_.get()] = new Napi::ObjectReference();
        g_widget_refs[button_.get()]->Reset(info.This().As<Napi::Object>(), 1);
    }

    ~ButtonWrap() {
        if (button_) {
            auto it = g_widget_refs.find(button_.get());
            if (it != g_widget_refs.end()) {
                delete it->second;
                g_widget_refs.erase(it);
            }
        }
    }

    lui::Widget* GetWidget() { return button_.get(); }

private:
    Napi::Value SetName(const Napi::CallbackInfo& info) {
        if (info.Length() < 1 || !info[0].IsString()) {
            Napi::TypeError::New(info.Env(), "String expected").ThrowAsJavaScriptException();
            return info.Env().Undefined();
        }
        button_->set_name(info[0].As<Napi::String>().Utf8Value());
        return info.This();
    }

    Napi::Value GetName(const Napi::CallbackInfo& info) {
        return Napi::String::New(info.Env(), button_->name());
    }

    Napi::Value SetText(const Napi::CallbackInfo& info) {
        if (info.Length() < 1 || !info[0].IsString()) {
            Napi::TypeError::New(info.Env(), "String expected").ThrowAsJavaScriptException();
            return info.Env().Undefined();
        }
        button_->set_text(info[0].As<Napi::String>().Utf8Value());
        return info.This();
    }

    Napi::Value GetText(const Napi::CallbackInfo& info) {
        return Napi::String::New(info.Env(), button_->text().c_str());
    }

    Napi::Value SetBounds(const Napi::CallbackInfo& info) {
        if (info.Length() < 4) {
            Napi::TypeError::New(info.Env(), "4 numbers expected").ThrowAsJavaScriptException();
            return info.Env().Undefined();
        }
        button_->set_bounds(
            info[0].As<Napi::Number>().Int32Value(),
            info[1].As<Napi::Number>().Int32Value(),
            info[2].As<Napi::Number>().Int32Value(),
            info[3].As<Napi::Number>().Int32Value()
        );
        return info.This();
    }

    Napi::Value SetVisible(const Napi::CallbackInfo& info) {
        if (info.Length() < 1 || !info[0].IsBoolean()) {
            Napi::TypeError::New(info.Env(), "Boolean expected").ThrowAsJavaScriptException();
            return info.Env().Undefined();
        }
        button_->set_visible(info[0].As<Napi::Boolean>().Value());
        return info.This();
    }

    Napi::Value OnClick(const Napi::CallbackInfo& info) {
        if (info.Length() < 1 || !info[0].IsFunction()) {
            Napi::TypeError::New(info.Env(), "Function expected").ThrowAsJavaScriptException();
            return info.Env().Undefined();
        }
        
        // Store the callback
        callback_ = std::make_unique<Napi::FunctionReference>();
        *callback_ = Napi::Persistent(info[0].As<Napi::Function>());
        
        // Set up the C++ callback
        button_->on_clicked = [this]() {
            if (callback_) {
                callback_->Call({});
            }
        };
        
        return info.This();
    }

private:
    std::unique_ptr<lui::TextButton> button_;
    std::unique_ptr<Napi::FunctionReference> callback_;
};

// ============================================================================
// Main Context Wrapper
// ============================================================================
class MainWrap : public Napi::ObjectWrap<MainWrap> {
public:
    static Napi::Object Init(Napi::Env env, Napi::Object exports) {
        Napi::Function func = DefineClass(env, "Main", {
            InstanceMethod("elevate", &MainWrap::Elevate),
            InstanceMethod("loop", &MainWrap::Loop),
            InstanceMethod("running", &MainWrap::Running),
            InstanceMethod("exitCode", &MainWrap::ExitCode),
            InstanceMethod("setExitCode", &MainWrap::SetExitCode),
        });

        exports.Set("Main", func);
        return exports;
    }

    MainWrap(const Napi::CallbackInfo& info) 
        : Napi::ObjectWrap<MainWrap>(info) {
        if (!g_main) {
            g_main = std::make_unique<lui::Main>(
                lui::Mode::PROGRAM, 
                std::make_unique<lui::OpenGL>()
            );
        }
    }

    ~MainWrap() {
        // Keep g_main alive for the process lifetime
    }

private:
    Napi::Value Elevate(const Napi::CallbackInfo& info) {
        if (info.Length() < 1 || !info[0].IsObject()) {
            Napi::TypeError::New(info.Env(), "Widget expected").ThrowAsJavaScriptException();
            return info.Env().Undefined();
        }

        lui::Widget* widget = nullptr;
        
        // Try unwrapping as different widget types
        auto widget_wrap = Napi::ObjectWrap<WidgetWrap>::Unwrap(info[0].As<Napi::Object>());
        if (widget_wrap) {
            widget = widget_wrap->GetWidget();
        } else {
            auto button_wrap = Napi::ObjectWrap<ButtonWrap>::Unwrap(info[0].As<Napi::Object>());
            if (button_wrap) {
                widget = button_wrap->GetWidget();
            }
        }

        if (!widget) {
            Napi::TypeError::New(info.Env(), "Invalid widget").ThrowAsJavaScriptException();
            return info.Env().Undefined();
        }

        uint32_t flags = lui::View::RESIZABLE;
        if (info.Length() > 1 && info[1].IsNumber()) {
            flags = info[1].As<Napi::Number>().Uint32Value();
        }

        auto view = g_main->elevate(*widget, flags, 0);
        
        return Napi::Boolean::New(info.Env(), view != nullptr);
    }

    Napi::Value Loop(const Napi::CallbackInfo& info) {
        double timeout = 0.0;
        if (info.Length() > 0 && info[0].IsNumber()) {
            timeout = info[0].As<Napi::Number>().DoubleValue();
        }
        g_main->loop(timeout);
        return info.Env().Undefined();
    }

    Napi::Value Running(const Napi::CallbackInfo& info) {
        return Napi::Boolean::New(info.Env(), g_main->running());
    }

    Napi::Value ExitCode(const Napi::CallbackInfo& info) {
        return Napi::Number::New(info.Env(), g_main->exit_code());
    }

    Napi::Value SetExitCode(const Napi::CallbackInfo& info) {
        if (info.Length() < 1 || !info[0].IsNumber()) {
            Napi::TypeError::New(info.Env(), "Number expected").ThrowAsJavaScriptException();
            return info.Env().Undefined();
        }
        g_main->set_exit_code(info[0].As<Napi::Number>().Int32Value());
        return info.This();
    }
};

// ============================================================================
// Module initialization
// ============================================================================
Napi::Object Init(Napi::Env env, Napi::Object exports) {
    WidgetWrap::Init(env, exports);
    ButtonWrap::Init(env, exports);
    MainWrap::Init(env, exports);
    
    // Export View flags constants
    auto ViewFlags = Napi::Object::New(env);
    ViewFlags.Set("NONE", Napi::Number::New(env, 0));
    ViewFlags.Set("RESIZABLE", Napi::Number::New(env, lui::View::RESIZABLE));
    exports.Set("ViewFlags", ViewFlags);
    
    return exports;
}

NODE_API_MODULE(lui_node, Init)

} // namespace lui_node
