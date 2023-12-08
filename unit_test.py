import clip_tokenizer_py
from transformers import CLIPTokenizer

tokenizer = clip_tokenizer_py.CLIPTokenizer("vocab.txt")
hf_tokenizer = CLIPTokenizer.from_pretrained("openai/clip-vit-base-patch32")

res = tokenizer.tokenize(["hello world"])
hf_res = hf_tokenizer("hello world", return_tensors="pt")

assert res.tokens == hf_res.input_ids.tolist()
assert res.attention_mask == hf_res.attention_mask.tolist()


# test russian
res = tokenizer.tokenize(["Привет мир"])
hf_res = hf_tokenizer("Привет мир", return_tensors="pt")
print(res.tokens)
print(hf_res.input_ids)

assert res.tokens == hf_res.input_ids.tolist()
assert res.attention_mask == hf_res.attention_mask.tolist()

## test chinese
res = tokenizer.tokenize(["你好世界"])
hf_res = hf_tokenizer("你好世界", return_tensors="pt")

assert res.tokens == hf_res.input_ids.tolist()
assert res.attention_mask == hf_res.attention_mask.tolist()

## test japanese
res = tokenizer.tokenize(["こんにちは世界"])
hf_res = hf_tokenizer("こんにちは世界", return_tensors="pt")

assert res.tokens == hf_res.input_ids.tolist()
assert res.attention_mask == hf_res.attention_mask.tolist()


# test hindi
res = tokenizer.tokenize(["नमस्ते दुनिया"])
hf_res = hf_tokenizer("नमस्ते दुनिया", return_tensors="pt")
print(res.tokens)
print(hf_res.input_ids)

assert res.tokens == hf_res.input_ids.tolist()
assert res.attention_mask == hf_res.attention_mask.tolist()

## test arabic
res = tokenizer.tokenize(["مرحبا بالعالم"])
hf_res = hf_tokenizer("مرحبا بالعالم", return_tensors="pt")

assert res.tokens == hf_res.input_ids.tolist()
assert res.attention_mask == hf_res.attention_mask.tolist()

## test korean
res = tokenizer.tokenize(["안녕하세요"])
hf_res = hf_tokenizer("안녕하세요", return_tensors="pt")

assert res.tokens == hf_res.input_ids.tolist()
assert res.attention_mask == hf_res.attention_mask.tolist()

## test spanish
res = tokenizer.tokenize(["Hola Mundo"])
hf_res = hf_tokenizer("Hola Mundo", return_tensors="pt")
print(res.tokens)
print(hf_res.input_ids)

assert res.tokens == hf_res.input_ids.tolist()
assert res.attention_mask == hf_res.attention_mask.tolist()

## test turkish
res = tokenizer.tokenize(["Merhaba Dünya"])
hf_res = hf_tokenizer("Merhaba Dünya", return_tensors="pt")

assert res.tokens == hf_res.input_ids.tolist()
assert res.attention_mask == hf_res.attention_mask.tolist()